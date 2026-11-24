#!/bin/env python3

from __future__ import annotations

import re
import sys
from pathlib import Path

import awkward as ak
import lh5
import numpy as np
from remage import remage_run
from remage.utils import fnv1a_name_hash

macro = sys.argv[1]
nthread = int(sys.argv[2])
mode = sys.argv[3]

assert mode in ("mt", "mp", "st")
assert mode != "st" or nthread == 1

output_stem = macro + (f"-{mode}" if nthread > 1 else "")
output_lh5 = f"{output_stem}.lh5"

# clean up files from last run(s).
for old_f in Path().glob(f"{output_stem}*.lh5"):
    Path(old_f).unlink()

# "parse" the macro.
pos_input_file = None
kin_input_file = None
with Path(f"macros/{macro}.mac").open("r") as macro_file:
    for line in macro_file:
        m = re.match(r"/RMG/Generator/FromFile/FileName (.*)\n$", line)
        if m:
            kin_input_file = m.group(1)
        m = re.match(r"/RMG/Generator/Confinement/FromFile/FileName (.*)\n$", line)
        if m:
            pos_input_file = m.group(1)

extra_args = {}
if mode in ("st", "mt"):
    extra_args["threads"] = nthread
    n_events = 10000
else:
    extra_args["procs"] = nthread
    n_events = int(10000 / nthread)

if "kin2" in macro or "singletable2" in macro:
    # when we have two particles per event, we only have half of the events.
    n_events //= 2

# special mode, in which we get each vertex twice
multi_vertex = "multivertex" in macro

# run remage, produce lh5 output.
files = remage_run(
    f"macros/{macro}.mac",
    macro_substitutions={"events": n_events},
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    log_level="summary",
    **extra_args,
)[1].get("output")
if len(files) == 1:
    files = files[0]

# validate output against input files.
if "pos" in macro:
    pos_input_file = pos_input_file.replace(".hdf5", ".lh5")
    input_pos = lh5.read("vtx/pos", lh5_file=pos_input_file).view_as("pd")
    output_pos = lh5.read("vtx", lh5_file=files).view_as("pd")

    output_pos = output_pos.sort_values("xloc")  # sort by linear column.
    output_pos = output_pos[["xloc", "yloc", "zloc"]]
    uniq, cnt = np.unique(output_pos["xloc"], return_counts=True)
    max_count = 1 if not multi_vertex else 2
    if not np.all(cnt <= max_count):
        msg = f"non-unique pos 'indices' {uniq[cnt > 1]}"
        raise ValueError(msg)

    output_pos = output_pos.to_numpy()

    input_pos = input_pos[["xloc", "yloc", "zloc"]]
    input_pos = input_pos.to_numpy()

    # re-scale to accommodate for different units.
    if "vtx-pos-mm.lh5" in pos_input_file:
        input_pos /= 1000

    def _compare_input_output(output_pos, input_pos):
        return np.all(np.isclose(output_pos, input_pos[0 : len(output_pos)]))

    if not multi_vertex:
        assert _compare_input_output(output_pos, input_pos)
    else:
        # the first and second particle emitted in each event are interleaved in the
        # output file and should have the same position.
        assert _compare_input_output(output_pos[0::2], input_pos)
        assert _compare_input_output(output_pos[1::2], input_pos)

if "volume" in macro:

    def _inside(pos, center_x, half, tol=0):
        return (
            (np.abs(pos[:, 0] - center_x) < half[0] + tol)
            & (np.abs(pos[:, 1]) < half[1] + tol)
            & (np.abs(pos[:, 2]) < half[2] + tol)
        )

    # analytic placement of the volumes in gdml/geometry.gdml with x >= 0 (in m),
    # ordered from mother to daughter.
    volumes = {
        "world_PV": (0, (1, 1, 1)),  # default name of the world physical volume.
        "optdet2": (0.27, (0.26, 0.51, 0.51)),
        "scint2": (0.27, (0.25, 0.5, 0.5)),
        "det2": (0.27, (0.05, 0.25, 0.25)),
        "calo2": (0.17, (0.0025, 0.15, 0.15)),
    }

    vtx = lh5.read("vtx", lh5_file=files).view_as("pd")
    pos = vtx[["xloc", "yloc", "zloc"]].to_numpy()
    assert np.all(pos[:, 0] >= 0)

    expected = np.full(len(vtx), "", dtype=object)
    # vertices on a volume surface (the x input positions are on a regular grid, which
    # coincides with some faces) can be attributed to either volume.
    on_surface = np.zeros(len(vtx), dtype=bool)
    for name, (center_x, half) in volumes.items():
        expected[_inside(pos, center_x, half)] = name
        on_surface |= _inside(pos, center_x, half, 1e-9) != _inside(
            pos, center_x, half, -1e-9
        )
    assert np.all(expected != "")
    assert np.sum(on_surface) < 0.01 * len(vtx)

    # check the name <-> id mapping (also across threads and processes).
    vol_tbl = lh5.read("vtx_volumes", lh5_file=files).view_as("pd")
    vol_map = {}
    for vol_id, vol_name_b in zip(vol_tbl["volumeid"], vol_tbl["name"], strict=True):
        vol_name = vol_name_b.decode()
        assert vol_map.setdefault(vol_name, vol_id) == vol_id
        assert vol_id == fnv1a_name_hash(vol_name, fold=False)
    # only volumes that contained a vertex are listed.
    assert set(expected[~on_surface]) <= set(vol_map) <= set(volumes)

    # check the volume of each vertex.
    expected_ids = np.array([vol_map[name] for name in expected[~on_surface]])
    assert np.all(vtx["volumeid"].to_numpy()[~on_surface] == expected_ids)

if "kin" in macro:
    kin_input_file = kin_input_file.replace(".hdf5", ".lh5")
    input_kin = lh5.read_as("vtx/kin", kin_input_file, "ak")
    output_kin = lh5.read_as("particles", files, "ak")

    output_kin = output_kin[ak.argsort(output_kin["ekin"])]  # sort by linear column.
    uniq, cnt = np.unique(output_kin["px"].to_numpy(), return_counts=True)
    if not np.all(cnt <= 1):
        msg = f"non-unique kin 'indices' {uniq[cnt > 1]}"
        raise ValueError(msg)

    # re-scale to accommodate for different convention (absolute momentum vs direction).
    output_p_scale = np.sqrt(
        output_kin["px"] ** 2 + output_kin["py"] ** 2 + output_kin["pz"] ** 2
    )
    for p_field in ("px", "py", "pz"):
        output_kin[p_field] = output_kin[p_field] / output_p_scale

    input_kin = input_kin[0 : len(output_kin)]

    cols_to_compare = {
        "px": "px",
        "py": "py",
        "pz": "pz",
        "ekin": "ekin",
        "g4_pid": "particle",
    }
    for col_in, col_out in cols_to_compare.items():
        assert np.all(
            np.isclose(output_kin[col_out].to_numpy(), input_kin[col_in].to_numpy())
        )

    # compare number of particles in the events
    part_in = input_kin["n_part"][input_kin["n_part"] > 0]
    part_out = ak.run_lengths(output_kin["evtid"])
    assert ak.all(part_in == part_out)

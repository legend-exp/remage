#!/bin/env python3

from __future__ import annotations

import re
import sys
from pathlib import Path

import numpy as np
from lgdo import lh5
from remage import remage_run

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
    if not np.all(cnt <= 1):
        msg = f"non-unique pos 'indices' {uniq[cnt > 1]}"
        raise ValueError(msg)

    input_pos = input_pos.iloc[0 : len(output_pos)]
    input_pos = input_pos[["xloc", "yloc", "zloc"]]

    # re-scale to accommodate for different units.
    if "vtx-pos-mm.lh5" in pos_input_file:
        input_pos /= 1000

    assert np.all(np.isclose(output_pos.to_numpy(), input_pos.to_numpy()))

if "kin" in macro:
    kin_input_file = kin_input_file.replace(".hdf5", ".lh5")
    input_kin = lh5.read("vtx/kin", lh5_file=kin_input_file).view_as("pd")
    output_kin = lh5.read("particles", lh5_file=files).view_as("pd")

    output_kin = output_kin.sort_values("ekin")  # sort by linear column.
    output_kin = output_kin[["px", "py", "pz", "ekin", "particle"]]
    uniq, cnt = np.unique(output_kin["px"], return_counts=True)
    if not np.all(cnt <= 1):
        msg = f"non-unique kin 'indices' {uniq[cnt > 1]}"
        raise ValueError(msg)

    output_p_scale = np.sqrt(
        output_kin["px"] ** 2 + output_kin["py"] ** 2 + output_kin["pz"] ** 2
    )
    output_kin["px"] /= output_p_scale
    output_kin["py"] /= output_p_scale
    output_kin["pz"] /= output_p_scale

    input_kin = input_kin.iloc[0 : len(output_kin)]
    input_kin = input_kin[["px", "py", "pz", "ekin", "g4_pid"]]

    assert np.all(np.isclose(output_kin.to_numpy(), input_kin.to_numpy()))

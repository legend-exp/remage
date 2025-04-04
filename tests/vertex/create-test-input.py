#!/bin/env python3
from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from lgdo import Array, Table, lh5

rmg_from_lh5 = sys.argv[1]


def _convert_lh5_to_hdf5(lh5_fn):
    if rmg_from_lh5 == "false":
        return

    orig_path = Path(lh5_fn)
    new_path = orig_path.with_suffix(".hdf5")
    shutil.copy(orig_path, new_path)
    ret = subprocess.run(
        [rmg_from_lh5, "--ntuple-group", "vtx", "--", str(new_path)], check=False
    )
    assert ret.returncode == 0


INPUT_FILE_ROWS = int(1e4)
rng = np.random.default_rng(123456)

# ==================================================
# position input


def _create_pos_input(dtype, vtx_pos_file, units):
    attrs = {"units": units}
    xloc = Array(np.linspace(0, 1000, INPUT_FILE_ROWS).astype(dtype), attrs=attrs)
    yloc = Array(rng.uniform(-1, 1, INPUT_FILE_ROWS).astype(dtype), attrs=attrs)
    zloc = Array(rng.uniform(-1, 1, INPUT_FILE_ROWS).astype(dtype), attrs=attrs)

    lh5.write(
        Table({"xloc": xloc, "yloc": yloc, "zloc": zloc}),
        "vtx/pos",
        lh5_file=vtx_pos_file,
        wo_mode="of",
    )
    _convert_lh5_to_hdf5(vtx_pos_file)


_create_pos_input(np.float64, "macros/vtx-pos.lh5", "m")
_create_pos_input(np.float64, "macros/vtx-pos-mm.lh5", "mm")

# ==================================================
# kinetics input.

p_theta = np.acos(rng.uniform(-1, 1, INPUT_FILE_ROWS))
p_phi = rng.uniform(0, 2 * np.pi, INPUT_FILE_ROWS)

px = Array(np.cos(p_phi) * np.sin(p_theta))
py = Array(np.sin(p_phi) * np.sin(p_theta))
pz = Array(np.cos(p_theta))

pdg = Array(np.ones(INPUT_FILE_ROWS, dtype=np.int64) * 11)
ekin = Array(np.linspace(1, 10, INPUT_FILE_ROWS), attrs={"units": "MeV"})

vtx_kin_file = "macros/vtx-kin.lh5"
lh5.write(
    Table({"g4_pid": pdg, "ekin": ekin, "px": px, "py": py, "pz": pz}),
    "vtx/kin",
    lh5_file=vtx_kin_file,
    wo_mode="of",
)
_convert_lh5_to_hdf5(vtx_kin_file)

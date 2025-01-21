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
        [rmg_from_lh5, "--ntuple-group", "vtx", "--", new_path], check=False
    )
    assert ret.returncode == 0


# ==================================================
# position input

xloc = Array(np.array([0, 1, 2] * 100, dtype=np.float64), attrs={"units": "m"})
yloc = Array(np.array([3, 4, 5] * 100, dtype=np.float64), attrs={"units": "m"})
zloc = Array(np.array([6, 7, 8] * 100, dtype=np.float64), attrs={"units": "m"})

vtx_pos_file = "macros/vtx-pos.lh5"
lh5.write(
    Table({"xloc": xloc, "yloc": yloc, "zloc": zloc}),
    "vtx/pos",
    lh5_file=vtx_pos_file,
    wo_mode="of",
)
_convert_lh5_to_hdf5(vtx_pos_file)

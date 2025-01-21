#!/bin/env python3

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import numpy as np
from lgdo import lh5

rmg = sys.argv[1]
macro = sys.argv[2]
nthread = int(sys.argv[3])

output_stem = macro + ("-mt" if nthread > 1 else "")
output_lh5 = f"{output_stem}.lh5"

# clean up files from last run(s).
for old_f in Path().glob(f"{output_stem}*.lh5"):
    Path(old_f).unlink()

# run remage, produce lh5 output.
cmd = [
    rmg,
    "-g",
    "gdml/geometry.gdml",
    "-o",
    output_lh5,
    "-t",
    str(nthread),
    "-l",
    "summary",
    "--",
    f"macros/{macro}.mac",
]
proc = subprocess.run(cmd, check=False)

if proc.returncode not in [0, 2]:
    print("remage subprocess failed")
    sys.exit(proc.returncode)

# list of output files.
files = output_lh5
if nthread > 1:
    files = [f"{output_stem}_t{i}.lh5" for i in range(nthread)]

# validate output against input files.
if "pos" in macro:
    input_pos = lh5.read("vtx/pos", lh5_file="macros/vtx-pos.lh5").view_as("pd")
    output_pos = lh5.read("stp/vertices", lh5_file=files).view_as("pd")

    output_pos = output_pos.sort_values("xloc")  # sort by linear column.
    output_pos = output_pos[["xloc", "yloc", "zloc"]]
    uniq, cnt = np.unique(output_pos["xloc"], return_counts=True)
    if not np.all(cnt <= 1):
        raise ValueError(uniq[cnt > 1])

    input_pos = input_pos.iloc[0 : len(output_pos)][["xloc", "yloc", "zloc"]]

    assert np.all(np.isclose(output_pos.to_numpy(), input_pos.to_numpy()))

if "kin" in macro:
    input_kin = lh5.read("vtx/kin", lh5_file="macros/vtx-kin.lh5").view_as("pd")
    output_kin = lh5.read("stp/particles", lh5_file=files).view_as("pd")

    output_kin = output_kin.sort_values("ekin")  # sort by linear column.
    output_kin = output_kin[["px", "py", "pz", "ekin", "particle"]]
    uniq, cnt = np.unique(output_kin["px"], return_counts=True)
    if not np.all(cnt <= 1):
        raise ValueError(uniq[cnt > 1])

    output_p_scale = np.sqrt(
        output_kin["px"] ** 2 + output_kin["py"] ** 2 + output_kin["pz"] ** 2
    )
    output_kin["px"] /= output_p_scale
    output_kin["py"] /= output_p_scale
    output_kin["pz"] /= output_p_scale

    input_kin = input_kin.iloc[0 : len(output_kin)]
    input_kin = input_kin[["px", "py", "pz", "ekin", "g4_pid"]]

    assert np.all(np.isclose(output_kin.to_numpy(), input_kin.to_numpy()))

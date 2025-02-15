#!/bin/env python3

from __future__ import annotations

import subprocess
import sys

from lgdo import lh5

rmg = sys.argv[1]
macro = "macros/gammafilter.mac"
output_lh5 = "gammafilter.lh5"

# run remage, produce lh5 output.
subprocess.run(
    [rmg, "-g", "gdml/geometry.gdml", "-o", output_lh5, "-w", "--", macro],
    check=False,
)

# check that we get to stable isotopes.
tracks = lh5.read("stp/tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[11] != 0  # primaries.
assert particle_numbers.get(22, 0) == 0  # gammas

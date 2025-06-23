#!/bin/env python3

from __future__ import annotations

from lgdo import lh5
from remage import remage_run

macro = "macros/keepprocess.mac"
output_lh5 = "keepprocess.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# check that we get all antineutrinos.
tracks = lh5.read("tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[-12] == 20

#!/bin/env python3

from __future__ import annotations

from lgdo import lh5
from remage import remage_run

macro = "macros/keepvolume-outside.mac"
output_lh5 = "keepvolume-outside.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# If we are outside of the keep volume we expect similar results to being inside the kill volume.
tracks = lh5.read("tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[1000902280] == 10  # primaries.
# In this test we also kill the radium, to make sure all alphas are contained in the first volume.
assert particle_numbers.get(1000882240, 0) == 0
assert particle_numbers.get(1000020040, 0) == 0  # alphas


macro = "macros/keepvolume-inside.mac"
output_lh5 = "keepvolume-inside.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# If we are inside the keepvolume, we expect similar results to being outside the kill volume.
tracks = lh5.read("tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[1000902280] == 10  # primaries.
assert particle_numbers[1000822080] == 10  # we should always get to Pb208.
assert particle_numbers[1000020040] != 0  # alphas

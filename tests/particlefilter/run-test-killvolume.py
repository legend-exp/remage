#!/bin/env python3

from __future__ import annotations

from lgdo import lh5
from remage import remage_run

macro = "macros/killvolume-inside.mac"
output_lh5 = "killvolume-inside.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# check that no alphas are in the output
tracks = lh5.read("stp/tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[1000902280] == 10  # primaries.
# In this test we also kill the radium, to make sure all alphas are contained in the first volume.
assert particle_numbers.get(1000882240, 0) == 0
assert particle_numbers.get(1000020040, 0) == 0  # alphas


macro = "macros/killvolume-outside.mac"
output_lh5 = "killvolume-outside.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# Here we expect everything to be normal. We do not know how many alphas we will get, but we should get some.
tracks = lh5.read("stp/tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[1000902280] == 10  # primaries.
assert particle_numbers[1000822080] == 10  # we should always get to Pb208.
assert particle_numbers[1000020040] != 0  # alphas

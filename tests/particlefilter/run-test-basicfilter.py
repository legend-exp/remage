#!/bin/env python3

from __future__ import annotations

from lgdo import lh5
from remage import remage_run

macro = "macros/gammafilter.mac"
output_lh5 = "gammafilter.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# check that no gammas are in the output
tracks = lh5.read("tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[11] != 0  # primaries.
assert particle_numbers.get(22, 0) == 0  # gammas


macro = "macros/alphafilter.mac"
output_lh5 = "alphafilter.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# check that no alphas are in the output, but still we end where expected.
tracks = lh5.read("tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[1000902280] == 10  # primaries.
assert particle_numbers[1000822080] == 10  # we should always get to Pb208.
assert particle_numbers.get(1000020040, 0) == 0  # alphas

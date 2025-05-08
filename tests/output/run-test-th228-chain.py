#!/bin/env python3

from __future__ import annotations

from lgdo import lh5
from remage import remage_run

macro = "macros/th228-chain.mac"
output_lh5 = "th228-chain.lh5"

# run remage, produce lh5 output.
remage_run(
    macro,
    gdml_files="gdml/geometry-box.gdml",
    output=output_lh5,
    flat_output=True,
    overwrite_output=True,
)

# check that we get to stable isotopes.
tracks = lh5.read("stp/tracks", output_lh5).view_as("pd")
particle_numbers = tracks["particle"].value_counts()
assert particle_numbers[1000902280] == 1000  # primaries.
assert particle_numbers[1000822080] == 1000  # we should always get to Pb208.

# check that we had some common processes.
processes = lh5.read("stp/processes", output_lh5)["name"].view_as("np")
assert b"Radioactivation" in processes or b"RadioactiveDecay" in processes
assert b"eBrem" in processes
assert b"eIoni" in processes
assert b"phot" in processes
assert b"compt" in processes

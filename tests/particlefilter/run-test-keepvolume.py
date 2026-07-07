#!/bin/env python3

from __future__ import annotations

from particlefilter_utils import run_and_count

# If we are outside of the keep volume we expect similar results to being inside the kill volume.
particle_numbers = run_and_count(
    "macros/keepvolume-outside.mac", "keepvolume-outside.lh5"
)
assert particle_numbers[1000902280] == 10  # primaries.
# In this test we also kill the radium, to make sure all alphas are contained in the first volume.
assert particle_numbers.get(1000882240, 0) == 0
assert particle_numbers.get(1000020040, 0) == 0  # alphas

# If we are inside the keepvolume, we expect similar results to being outside the kill volume.
particle_numbers = run_and_count(
    "macros/keepvolume-inside.mac", "keepvolume-inside.lh5"
)
assert particle_numbers[1000902280] == 10  # primaries.
assert particle_numbers[1000822080] == 10  # we should always get to Pb208.
assert particle_numbers[1000020040] != 0  # alphas

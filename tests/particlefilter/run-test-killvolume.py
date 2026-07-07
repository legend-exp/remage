#!/bin/env python3

from __future__ import annotations

from particlefilter_utils import run_and_count

# check that no alphas are in the output
particle_numbers = run_and_count(
    "macros/killvolume-inside.mac", "killvolume-inside.lh5"
)
assert particle_numbers[1000902280] == 10  # primaries.
# In this test we also kill the radium, to make sure all alphas are contained in the first volume.
assert particle_numbers.get(1000882240, 0) == 0
assert particle_numbers.get(1000020040, 0) == 0  # alphas

# Here we expect everything to be normal. We do not know how many alphas we will get, but we should get some.
particle_numbers = run_and_count(
    "macros/killvolume-outside.mac", "killvolume-outside.lh5"
)
assert particle_numbers[1000902280] == 10  # primaries.
assert particle_numbers[1000822080] == 10  # we should always get to Pb208.
assert particle_numbers[1000020040] != 0  # alphas

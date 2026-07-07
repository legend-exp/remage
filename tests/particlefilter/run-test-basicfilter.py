#!/bin/env python3

from __future__ import annotations

from particlefilter_utils import run_and_count

# check that no gammas are in the output
particle_numbers = run_and_count("macros/gammafilter.mac", "gammafilter.lh5")
assert particle_numbers[11] != 0  # primaries.
assert particle_numbers.get(22, 0) == 0  # gammas

# check that no alphas are in the output, but still we end where expected.
particle_numbers = run_and_count("macros/alphafilter.mac", "alphafilter.lh5")
assert particle_numbers[1000902280] == 10  # primaries.
assert particle_numbers[1000822080] == 10  # we should always get to Pb208.
assert particle_numbers.get(1000020040, 0) == 0  # alphas

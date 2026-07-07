#!/bin/env python3

from __future__ import annotations

from particlefilter_utils import run_and_count

# check that we get all antineutrinos.
particle_numbers = run_and_count("macros/killprocess.mac", "killprocess.lh5")
assert -12 not in particle_numbers

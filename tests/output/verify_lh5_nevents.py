from __future__ import annotations

import sys

import lh5

file = sys.argv[2]
expected = int(sys.argv[1])

try:
    n_ev = lh5.read("/number_of_events", file)
    if n_ev.value != expected:
        msg = f"event count {n_ev.value} in file {file} does not match expectation {expected}"
        raise RuntimeError(msg)
except lh5.io.exceptions.LH5DecodeError:
    # not present (but already checked through the other parts of this test)
    pass

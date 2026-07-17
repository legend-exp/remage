"""Tests for the interaction of user-set RNG seeds with process-based parallelization.

The python wrapper hands every worker process an identical command line and macro, so a seed
requested in the macro would be shared by all of them and each worker would simulate the very same
events -- while still writing them out under distinct filenames and event ids, making the
duplication invisible in the merged output.

These tests pin down the user-facing contract:
  1. same seed          -> reproducible across runs (also with -P),
  2. workers of one run -> must NOT all simulate the same events,
  3. single process     -> keeps using the user seed verbatim.
"""

from __future__ import annotations

from pathlib import Path

import awkward as ak
import numpy as np
from lh5 import read_as
from remage import remage_run

SEED = 1403045780
N_EVENTS = 500
N_PROCS = 2

MACRO = f"""
/RMG/Geometry/RegisterDetector Germanium germanium 001

/RMG/Manager/Randomization/Seed {SEED}

/run/initialize

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/particle gamma
/gps/position 0 0 0
/gps/ang/type iso
/gps/energy 500 keV

/run/beamOn {N_EVENTS}
"""


def _run(tmptestdir: Path, tag: str, *, procs: int = 1) -> list[np.ndarray]:
    """Run the macro and return the per-process germanium energy depositions."""
    out = tmptestdir / f"{tag}.lh5"
    for old in tmptestdir.glob(f"{tag}*.lh5"):
        old.unlink()

    files = remage_run(
        MACRO,
        gdml_files="gdml/geometry.gdml",
        output=out,
        procs=procs,
        log_level="warning",
    )[1].get("output")
    if not isinstance(files, list):
        files = [files]
    assert len(files) == procs, f"expected {procs} output file(s), got {files}"

    # sort by filename, so that the "_pN" suffix fixes the order across runs.
    # post-processing reshapes the steps into one (jagged) entry per event.
    return [
        ak.to_numpy(ak.flatten(read_as("stp/det001", f, "ak").edep))
        for f in sorted(files)
    ]


def test_multiproc_is_reproducible(tmptestdir):
    """A fixed seed must stay reproducible in multi-processing mode.

    Deriving a per-process seed must not make runs random: rerunning the identical command has to
    reproduce every worker bit-for-bit.
    """
    a = _run(tmptestdir, "repro-a", procs=N_PROCS)
    b = _run(tmptestdir, "repro-b", procs=N_PROCS)

    for i, (proc_a, proc_b) in enumerate(zip(a, b, strict=True)):
        assert np.array_equal(proc_a, proc_b), f"process {i} is not reproducible"


def test_multiproc_workers_are_not_identical(tmptestdir):
    """Workers must be seeded distinctly, else every event is simulated once per process."""
    edeps = _run(tmptestdir, "distinct", procs=N_PROCS)

    assert not np.array_equal(edeps[0], edeps[1]), (
        "all worker processes simulated identical events -- the macro seed is not "
        "being offset per process"
    )


def test_single_proc_uses_user_seed_verbatim(tmptestdir):
    """Without multi-processing the user seed must be used unchanged.

    Process 0 of a multi-processing run is defined to use it unchanged too, so its output has to
    agree with a plain single-process run of the same macro.
    """
    single = _run(tmptestdir, "single", procs=1)[0]
    proc0 = _run(tmptestdir, "multi", procs=N_PROCS)[0]

    assert np.array_equal(single, proc0)

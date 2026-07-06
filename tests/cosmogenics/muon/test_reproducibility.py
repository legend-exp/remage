"""
Regression test for GEN-6: the EcoMug cosmic-muon generator must honour remage's
RNG seed (/RMG/Manager/Randomization/Seed) like every other generator.

These tests pin down the user-facing contract:
  1. same seed  -> byte-identical primary muons,
  2. other seed -> different primary muons,
  3. multithreaded workers do NOT all produce the same muon sequence.
"""

from __future__ import annotations

from pathlib import Path

import awkward as ak
import numpy as np
import pyg4ometry as pg4
from lh5 import read_as
from remage import remage_run

# small hemisphere sky so a modest world box contains it (default radius is 50 m).
SKY_RADIUS_M = 5
WORLD_HALF_M = 8
N_EVENTS = 300

MACRO = """
/RMG/Output/ActivateOutputScheme Vertex

/run/initialize

/RMG/Output/Vertex/StorePrimaryParticleInformation true

/RMG/Manager/Randomization/Seed {seed}

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select CosmicMuons
/RMG/Generator/CosmicMuons/SkyShape Sphere
/RMG/Generator/CosmicMuons/SkyHSphereRadius {sky} m
/RMG/Generator/CosmicMuons/MomentumMin 0.5 GeV/c
/RMG/Generator/CosmicMuons/MomentumMax 10 GeV/c

/run/beamOn {events}
"""


def _make_geometry(path: Path) -> None:
    reg = pg4.geant4.Registry()
    air = pg4.geant4.MaterialPredefined("G4_AIR", registry=reg)
    world_s = pg4.geant4.solid.Box(
        "world",
        2 * WORLD_HALF_M,
        2 * WORLD_HALF_M,
        2 * WORLD_HALF_M,
        registry=reg,
        lunit="m",
    )
    world_l = pg4.geant4.LogicalVolume(world_s, air, "world", registry=reg)
    reg.setWorld(world_l)
    w = pg4.gdml.Writer()
    w.addDetector(reg)
    w.write(str(path))


def _run(seed: int, tag: str, *, threads: int = 1) -> np.ndarray:
    # clean up files from earlier runs, including per-worker "..._t0.lh5" outputs.
    for old in Path().glob(f"reproducibility-{tag}*.lh5"):
        old.unlink()

    gdml = Path(f"reproducibility-{tag}.gdml")
    _make_geometry(gdml)
    out = f"reproducibility-{tag}.lh5"

    extra = {"threads": threads} if threads > 1 else {}
    files = remage_run(
        MACRO.format(seed=seed, sky=SKY_RADIUS_M, events=N_EVENTS),
        gdml_files=str(gdml),
        output=out,
        flat_output=True,
        log_level="warning",
        **extra,
    )[1].get("output")
    if not isinstance(files, list):
        files = [files]

    # MT runs may write one file per worker; concatenate all primaries.
    particles = ak.concatenate([read_as("particles", f, "ak") for f in files])
    kin = np.stack(
        [
            particles["px"].to_numpy(),
            particles["py"].to_numpy(),
            particles["pz"].to_numpy(),
            particles["ekin"].to_numpy(),
        ],
        axis=1,
    )
    assert len(kin) == N_EVENTS, f"expected {N_EVENTS} primaries, got {len(kin)}"
    return kin


def test_same_seed_is_reproducible():
    """Same seed -> byte-identical muon kinematics (the core GEN-6 regression)."""
    a = _run(1234, "same-a")
    b = _run(1234, "same-b")
    assert np.array_equal(a, b)


def test_different_seed_differs():
    """A different seed must actually change the muons (guards a hardcoded-seed regression)."""
    a = _run(1234, "diff-a")
    b = _run(4321, "diff-b")
    # sort both so a mere reordering wouldn't count as a difference.
    a = a[np.lexsort(a.T)]
    b = b[np.lexsort(b.T)]
    assert not np.array_equal(a, b)


def test_mt_workers_are_not_identical():
    """In MT mode workers must be seeded distinctly, else every muon is duplicated per thread."""
    threads = 4
    kin = _run(1234, "mt", threads=threads)
    # if all workers shared one seed, each muon vector would recur ~once per thread.
    n_unique = len({tuple(row) for row in kin})
    assert n_unique > N_EVENTS / threads * 1.5

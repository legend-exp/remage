"""Byte-exact round-trip test for the custom (struct-based) optical-photon staging.

Only optical photons are checked here, on purpose: the flush/re-injection code
(``FlushToTempFile`` / ``ReinjectBatch`` / ``OpenScratchFile`` / ``ResetScratch``) is
templated over the record struct, so exercising it byte-exact for ``RMGPhotonState`` also
covers the ``RMGElectronState`` instantiation -- the only electron-specific difference is the
44-byte layout and ``push_electron``'s pdg->definition mapping. A separate byte-exact electron
test would re-run the same machinery for little added confidence. The electron staging path is
instead exercised under load (recording + scratch-file write) by the "electron" mode of
``test_muon_stress.py``.
"""

from __future__ import annotations

import awkward as ak
import numpy as np
from staging_test_utils import read_tracks, run_macro

# Optical-photon PDG code as remage records it in the track output.
OPTICAL_PHOTON = -22


def _staging_macro(
    *,
    seed: int,
    events: int,
    energy_mev: float,
    custom: bool,
    store_path: str | None = None,
    limit_mb: int | None = None,
) -> str:
    """Fire an electron into the liquid-argon world and defer every optical photon.

    ``custom`` selects which staging implementation handles the deferred photons:
      * ``False`` -> native Geant4 staging (``RMGDeferring`` off): the real G4Tracks are
        parked on the waiting stack and promoted in a single native stage. This keeps
        double precision and is the physics reference.
      * ``True``  -> remage's struct-based staging: each photon is recorded into a
        fixed-size float32 POD struct, killed, and rebuilt on re-injection. The storage
        backend is then chosen by ``store_path``/``limit_mb``:
          - both ``None``   -> records buffered in RAM, uncapped (no scratch file)
          - ``limit_mb = 1`` + ``store_path`` -> spill to a scratch file past ~1 MB

    Keep-all deferral (no DiscardWaitingTracks... command) re-injects every staged photon,
    so the full machinery runs for every event.
    """
    lines = [
        f"/random/setSeeds {seed} {seed}",
        "/RMG/Geometry/RegisterDetector Germanium detector_phys 0",
        "/RMG/Output/ActivateOutputScheme Track",
        "/RMG/Output/ActivateOutputScheme Staging",
        "",
        "/RMG/Processes/OpticalPhysics true",
        "",
        "/run/initialize",
        "",
        "/RMG/Output/Track/StoreAlways true",
        "/RMG/Output/Track/StoreStageID true",
        "/RMG/Output/Track/StoreOpticalPhotons true",
        "",
        "/RMG/Staging/OpticalPhotons/DeferToWaitingStage true",
        f"/RMG/Staging/OpticalPhotons/RMGDeferring {'true' if custom else 'false'}",
    ]
    if store_path is not None:
        lines.append(f"/RMG/Staging/OpticalPhotons/StorePath {store_path}")
    if limit_mb is not None:
        lines.append(f"/RMG/Staging/OpticalPhotons/LimitMemory {limit_mb}")
    lines += [
        "",
        "/RMG/Generator/Confine UnConfined",
        "/RMG/Generator/Select GPS",
        "/gps/position 0 0 50 cm",
        "/gps/particle e-",
        f"/gps/energy {energy_mev} MeV",
        "/gps/direction 0 0 1",
        "",
        f"/run/beamOn {events}",
    ]
    return "\n".join(lines)


def _reinjected_photons(output: str):
    """Per-photon creation records of the re-injected optical photons.

    Every optical photon is deferred and killed/parked in stage 0, so all of them are
    re-injected and carry ``stageid >= 1`` (none is tracked in stage 0). The track output
    stores one row per track at creation, so these are exactly the values restored on
    re-injection, before any stage-1 tracking.

    Returned as a tuple ``(evtid, ekin, time, pos)`` sorted by the exact fields
    ``(evtid, ekin, time)``, so the same photon lands on the same row in every backend
    regardless of the (backend-dependent) re-injection order. Everything is at float32,
    the precision of the record struct.

    Note the precision at which each field can match the *native* double-precision run:
      * ``ekin`` (MeV) and ``time`` (ns) are stored in the same unit the output uses, so
        there is no unit conversion and they are exactly float32(value) in every backend.
      * position is stored as float32 in mm but written out in m, so the custom value is
        ``float32(mm)/1000`` vs the native ``double(mm)/1000`` -- a double-rounding that
        leaves them equal only to ~1 float32 ULP. RAM vs spill still match position
        exactly (identical struct bytes); only vs native is it approximate.
    """
    tracks = read_tracks(output)
    assert tracks is not None, f"missing track output in {output}"
    assert "stageid" in tracks.fields, "track output has no stageid column"

    p = tracks[(tracks.particle == OPTICAL_PHOTON) & (tracks.stageid >= 1)]
    evtid = ak.to_numpy(p.evtid).astype(np.int64)
    ekin = ak.to_numpy(p.ekin).astype(np.float32)
    time = ak.to_numpy(p.time).astype(np.float32)
    pos = np.column_stack(
        [
            ak.to_numpy(p.xloc).astype(np.float32),
            ak.to_numpy(p.yloc).astype(np.float32),
            ak.to_numpy(p.zloc).astype(np.float32),
        ]
    )
    # Momentum vector = direction * energy. The struct stores the direction as float32 and
    # re-injection renormalises it (dir.unit()), so vs native this carries a float32
    # truncation + renormalisation error on top of the energy; RAM vs spill is still exact.
    mom = np.column_stack(
        [
            ak.to_numpy(p.px).astype(np.float32),
            ak.to_numpy(p.py).astype(np.float32),
            ak.to_numpy(p.pz).astype(np.float32),
        ]
    )
    order = np.lexsort((time, ekin, evtid))
    return evtid[order], ekin[order], time[order], pos[order], mom[order]


def _unit(mom: np.ndarray) -> np.ndarray:
    """Unit momentum direction, computed in float64 to avoid float32 sqrt noise."""
    m = mom.astype(np.float64)
    return m / np.linalg.norm(m, axis=1, keepdims=True)


def _parent_ids(output: str) -> np.ndarray:
    tracks = read_tracks(output)
    p = tracks[(tracks.particle == OPTICAL_PHOTON) & (tracks.stageid >= 1)]
    return ak.to_numpy(p.parent_trackid)


def _max_stageid(output: str) -> int:
    tracks = read_tracks(output)
    return int(ak.max(tracks.stageid))


def test_custom_staging_reproduces_native_g4_staging():
    """remage's struct-based staging -- in RAM and spilled to a scratch file -- must
    reproduce native Geant4 waiting-stack staging, to the float32 precision of the record
    struct.

    Three backends handle the same deferred optical photons at the same seed:
      * native G4 waiting stack (real G4Tracks)  -- the double-precision reference,
      * remage custom staging, records in RAM,
      * remage custom staging, records spilled to a scratch file.

    Stage 0 is identical in all three (the photons are set aside, not tracked, either way),
    so the multiset of re-injected photons must match. Comparing the RAM and spill runs
    checks the scratch-file round-trip is exact; comparing both against the native run
    checks the float32 struct faithfully reconstructs the real tracks.

    Exactly one event, on purpose: the comparison is byte-exact (at float32), not
    statistical. A smaller memory limit re-injects across more stages, tracking the
    stage-1 photons in a different order and thus drawing differently from the shared event
    RNG; in sequential mode the engine is not re-seeded per event, so with more than one
    event a later shower would diverge between backends even though the re-injection
    round-trip is exact. The creation records compared here are written before any stage-1
    tracking, so within one event they are backend-independent.

    Optical photons are terminal in this WLS-free liquid argon (they Rayleigh-scatter and
    leave the world without re-emitting), so every re-injected photon stays a single track:
    there are no re-injection-stage-born daughters whose number would depend on order.
    """
    events = 1
    seed = 573137
    energy_mev = (
        2.0  # ~50k scintillation photons/event, well past the 1 MB spill threshold
    )

    native = "staging-g4-native.lh5"
    ram = "staging-rmg-ram.lh5"
    spill = "staging-rmg-spill.lh5"

    run_macro(
        _staging_macro(seed=seed, events=events, energy_mev=energy_mev, custom=False),
        native,
    )
    run_macro(
        _staging_macro(seed=seed, events=events, energy_mev=energy_mev, custom=True),
        ram,
    )
    run_macro(
        _staging_macro(
            seed=seed,
            events=events,
            energy_mev=energy_mev,
            custom=True,
            store_path=".",
            limit_mb=1,
        ),
        spill,
    )

    n_evt, n_ekin, n_time, n_pos, n_mom = _reinjected_photons(native)
    r_evt, r_ekin, r_time, r_pos, r_mom = _reinjected_photons(ram)
    s_evt, s_ekin, s_time, s_pos, s_mom = _reinjected_photons(spill)

    assert len(n_ekin) > 0, "no optical photons were re-injected in the native run"

    # The spill run must genuinely exercise the memory-bounded path: it re-injects across
    # many stages, while the RAM and native runs promote everything in a single stage. This
    # confirms the scratch file was used without hard-coding sizeof(struct)/flush counts.
    assert _max_stageid(native) == 1, "native staging unexpectedly used multiple stages"
    assert _max_stageid(ram) == 1, "in-RAM staging unexpectedly used multiple stages"
    assert _max_stageid(spill) > 1, (
        "spill run did not re-inject across multiple stages -- it never spilled; "
        "increase the source energy or lower LimitMemory"
    )

    # No photon lost, duplicated or added by either custom backend.
    assert len(r_ekin) == len(n_ekin), (
        f"RAM staging changed the photon count: {len(r_ekin)} vs {len(n_ekin)}"
    )
    assert len(s_ekin) == len(n_ekin), (
        f"spill changed the photon count: {len(s_ekin)} vs {len(n_ekin)}"
    )

    # 1) scratch-file round-trip is exact: identical records to keeping them in RAM, down
    #    to the position bits (same struct bytes) -- no record lost, duplicated or corrupted.
    np.testing.assert_array_equal(s_evt, r_evt)
    np.testing.assert_array_equal(s_ekin, r_ekin)
    np.testing.assert_array_equal(s_time, r_time)
    np.testing.assert_array_equal(s_pos, r_pos)
    np.testing.assert_array_equal(s_mom, r_mom)

    # 2) the custom float32 struct reproduces the native double-precision tracks: energy and
    #    time exactly, position and momentum direction to float32 precision (see
    #    _reinjected_photons on the mm/m double-rounding and the direction renormalisation).
    #    The tolerances are far below any field-mapping error.
    np.testing.assert_array_equal(r_evt, n_evt)
    np.testing.assert_array_equal(r_ekin, n_ekin)
    np.testing.assert_array_equal(r_time, n_time)
    np.testing.assert_allclose(r_pos, n_pos, rtol=0, atol=1e-5)
    np.testing.assert_allclose(_unit(r_mom), _unit(n_mom), rtol=0, atol=1e-5)

    # 3) lineage restored on re-injection: photons carry their parent id, not 0.
    assert _parent_ids(ram).min() > 0, "re-injected photons lost their parent id"

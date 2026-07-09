from __future__ import annotations

import awkward as ak
import lh5
import numpy as np
import pyg4ometry as pg
import pytest
from remage import remage_run

# Ar39 ground-state-to-ground-state beta- Q value (ENSDF).
Q_AR39 = 0.565  # MeV
# Mass of the K39 recoil nucleus, used to convert the residual momentum of the
# IB treatment into the energy that the nucleus implicitly absorbs.
M_K39 = 39 * 931.494  # MeV


def create_geometry():
    reg = pg.geant4.Registry()
    world_s = pg.geant4.solid.Box("world_s", 200, 200, 200, reg, lunit="cm")
    world_l = pg.geant4.LogicalVolume(world_s, "G4_Galactic", "world", reg)
    reg.setWorld(world_l)

    lar_s = pg.geant4.solid.Box("lar_s", 100, 100, 100, reg, lunit="cm")
    lar_l = pg.geant4.LogicalVolume(lar_s, "G4_lAr", "lar", reg)
    pg.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "lar", world_l, reg)

    return reg


def _get_macro(enable_ib, n_events=300) -> str:
    # A biasing factor this large drives the IB probability above 1 for every
    # beta: *every* decay emits an IB photon
    ib = (
        """
/RMG/Processes/EnableInnerBremsstrahlung true
/run/initialize
/RMG/Processes/InnerBremsstrahlung/BiasingFactor 1e6
"""
        if enable_ib
        else "/run/initialize"
    )

    return f"""
/RMG/Manager/Logging/LogLevel error
/RMG/Output/ActivateOutputScheme Track
{ib}
/RMG/Generator/Confine Volume
/RMG/Generator/Confinement/Physical/AddVolume lar
/RMG/Generator/Select GPS
/gps/particle ion
/gps/ion 18 39 0
/gps/energy 0 keV
/gps/ang/type iso
/run/beamOn {n_events}
"""


def _decay_products(tmp_path, enable_ib):
    """Run a short Ar39 simulation and sum the four-momenta of the decay products.

    Returns ``(e_tot, p_tot, n_gamma)``, each with one entry per event: the total
    kinetic energy of the direct daughters of the Ar39 nucleus (beta, neutrino,
    K39 recoil and any IB photon), their total momentum, and the number of
    photons emitted directly by the decay.
    """
    gdml = tmp_path / "geometry.gdml"
    writer = pg.gdml.Writer()
    writer.addDetector(create_geometry())
    writer.write(str(gdml))

    out = tmp_path / f"tracks_{enable_ib}.lh5"
    remage_run(
        macros=_get_macro(enable_ib),
        gdml_files=str(gdml),
        output=str(out),
        overwrite_output=True,
    )

    tracks = lh5.read_as("tracks", str(out), "ak")
    evtid, trackid, parent, pid, ekin, px, py, pz = (
        ak.to_numpy(tracks[c])
        for c in (
            "evtid",
            "trackid",
            "parent_trackid",
            "particle",
            "ekin",
            "px",
            "py",
            "pz",
        )
    )

    # The primary Ar39 nuclei, and the tracks they directly created. Restricting
    # to *direct* daughters excludes everything the beta does downstream
    # (ionisation, external bremsstrahlung), which is not part of the decay
    # energy balance.
    is_ar39 = pid == 1000180390
    assert is_ar39.sum() > 0
    ar39 = set(zip(evtid[is_ar39], trackid[is_ar39], strict=True))
    is_daughter = (
        np.fromiter(
            (p in ar39 for p in zip(evtid, parent, strict=True)), bool, len(evtid)
        )
        & ~is_ar39
    )

    n_evt = evtid.max() + 1

    def _sum(w):
        return np.bincount(evtid[is_daughter], weights=w[is_daughter], minlength=n_evt)

    e_tot = _sum(ekin)
    p_tot = np.stack([_sum(c) for c in (px, py, pz)], axis=-1)
    n_gamma = np.bincount(evtid[is_daughter & (pid == 22)], minlength=n_evt)

    return e_tot, p_tot, n_gamma


@pytest.fixture(scope="module")
def with_ib(tmp_path_factory):
    return _decay_products(tmp_path_factory.mktemp("ib"), True)


@pytest.fixture(scope="module")
def without_ib(tmp_path_factory):
    return _decay_products(tmp_path_factory.mktemp("noib"), False)


def test_ib_is_actually_exercised(with_ib, without_ib):
    """The energy tests below are only meaningful if IB photons were emitted."""
    assert (with_ib[2] > 0).mean() > 0.9
    assert (without_ib[2] > 0).sum() == 0


def test_energy_is_conserved(with_ib):
    """The IB photon must be paid for by the beta, not created out of nothing.

    The kinetic energy shared among the decay products is fixed by the Q value,
    event by event, so this is an exact per-event identity.
    """
    e_tot, _, n_gamma = with_ib

    assert np.ptp(e_tot) < 1e-6  # MeV
    assert e_tot.mean() == pytest.approx(Q_AR39, abs=3e-3)
    # in particular, events with an IB photon release no more energy than others
    assert e_tot[n_gamma > 0].mean() == pytest.approx(
        e_tot[n_gamma == 0].mean(), abs=1e-6
    )


def test_energy_balance_unaffected_by_ib(with_ib, without_ib):
    """IB redistributes the decay energy without changing the total."""
    assert with_ib[0].mean() == pytest.approx(without_ib[0].mean(), abs=1e-6)


def test_momentum_imbalance_is_negligible(with_ib, without_ib):
    """Without IB the decay closes in momentum exactly.

    With IB it cannot: the beta is put back on shell at the reduced kinetic
    energy, so only its direction is free and the residual momentum is left to
    the recoiling nucleus. Assert that this is a good approximation, i.e. that
    the energy the nucleus would take up is negligible against the Q value.
    """
    # the residual scale here is set by the single-precision momentum storage
    assert np.linalg.norm(without_ib[1], axis=-1).max() < 1e-5  # MeV/c

    residual = np.linalg.norm(with_ib[1], axis=-1)
    recoil = residual**2 / (2 * M_K39)
    assert recoil.max() < 1e-4  # MeV, i.e. below 100 eV and six orders under Q

from __future__ import annotations

import numpy as np
import pyg4ometry as pg
import pytest
import scipy as sp
from lgdo import lh5
from remage import remage_run


def create_geometry(material: str = "G4_lAr"):
    reg = pg.geant4.Registry()
    world_s = pg.geant4.solid.Box("world_s", 2000, 2000, 2000, reg, lunit="cm")
    world_l = pg.geant4.LogicalVolume(world_s, "G4_Galactic", "world", reg)
    reg.setWorld(world_l)

    lar_s = pg.geant4.solid.Box("lar_s", 1000, 1000, 1000, reg, lunit="cm")
    lar_l = pg.geant4.LogicalVolume(lar_s, material, "lar", reg)
    pg.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "lar", world_l, reg)

    return reg


def _write_macro(
    tmp_path,
    cut,
    particle,
    volume_material="G4_lAr",
    selective_particle="e-",
    energy=1.0,
):
    reg = create_geometry(volume_material)

    gdml_path = tmp_path / f"geometry_{particle}_{cut}.gdml"
    writer = pg.gdml.Writer()
    writer.addDetector(reg)
    writer.write(str(gdml_path))

    macro_path = tmp_path / f"macro_{particle}_{cut}.mac"
    macro_content = f"""
/RMG/Manager/Logging/LogLevel detail
/RMG/Geometry/SetEkinMinForParticle {cut} MeV lar {selective_particle}
/RMG/Processes/HadronicPhysics Shielding
/RMG/Geometry/RegisterDetector Scintillator lar 1
/tracking/verbose 1

/run/initialize

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/random/setSeeds 1 2

/RMG/Output/Scintillator/StoreParticleVelocities true
/RMG/Output/Scintillator/StoreTrackID true

/RMG/Generator/Select GPS
/gps/particle {particle}
/gps/energy {energy} MeV
/gps/position 0 0 0 cm
/gps/direction 1 0 0

/run/beamOn 1
"""
    macro_path.write_text(macro_content)
    return gdml_path, macro_path


def _read_steps(output_lh5):
    steps = lh5.read("stp/", str(output_lh5))
    assert steps is not None, "No step data found in output"
    assert "lar" in steps, "No detector volume data found in output"
    return steps["lar"]


@pytest.mark.parametrize("cut", [2.0, 0.6])
def test_selective_ekin_min_electrons(tmp_path, cut):
    """
    Electrons registered with SetEkinMinForParticle should obey the cut.

    For cut=2.0, a 1 MeV electron starts below threshold and should be killed immediately.
    For cut=0.6, a 1 MeV electron starts above threshold and should continue until it drops
    below the cut.
    """
    gdml_path, macro_path = _write_macro(tmp_path, cut=cut, particle="e-")

    output_lh5 = tmp_path / "output_electron.lh5"
    remage_run(
        str(macro_path),
        gdml_files=str(gdml_path),
        output=str(output_lh5),
        flat_output=True,
        overwrite_output=True,
    )

    det = _read_steps(output_lh5)

    c = sp.constants.physical_constants["speed of light in vacuum"][0]
    m = sp.constants.physical_constants["electron mass energy equivalent in MeV"][0]
    v_in_m_per_s_pre = det["v_pre"].view_as("ak") * 1e9
    v_in_m_per_s_post = det["v_post"].view_as("ak") * 1e9
    E_total_pre = m / np.sqrt(1.0 - v_in_m_per_s_pre**2 / c**2)
    E_total_post = m / np.sqrt(1.0 - v_in_m_per_s_post**2 / c**2)
    E_kin_pre = E_total_pre - m
    E_kin_post = E_total_post - m

    if cut == 2.0:
        max_ekin = np.max(E_kin_pre)
        num_hits = len(det.view_as("ak"))
        assert num_hits == 1, (
            f"Expected only one step recorded for cut={cut}, got {num_hits}"
        )
        assert max_ekin < cut, (
            f"For cut={cut}: expected all recorded electron energies below {cut} MeV, "
            f"got max={max_ekin:.3f} MeV"
        )
    else:
        primary_mask = det["trackid"].view_as("ak") == 1
        E_kin_pre_prim = E_kin_pre[primary_mask]
        E_kin_post_prim = E_kin_post[primary_mask]

        step_with_post_not_zero = E_kin_post_prim > 0
        min_ekin_pre = np.min(E_kin_pre_prim[step_with_post_not_zero])
        min_ekin_post = np.min(E_kin_post_prim[step_with_post_not_zero])

        assert min_ekin_pre > cut, (
            f"For cut={cut}: expected smallest pre-step energy > {cut} MeV, "
            f"got {min_ekin_pre:.3f} MeV"
        )
        assert min_ekin_post < cut, (
            f"For cut={cut}: expected smallest post-step energy < {cut} MeV, "
            f"got {min_ekin_post:.3f} MeV"
        )


@pytest.mark.parametrize(
    "particle", ["alpha", "mu-", "pi-", "proton", "neutron", "gamma"]
)
def test_selective_ekin_min_unregistered_particles_are_ignored(tmp_path, particle):
    """
    Particles not registered with SetEkinMinForParticle should not be cut.
    """
    energy = 1.0
    cut = 2.0
    if particle in ["alpha", "proton"]:
        energy = 100.0
        cut = 200.0
    gdml_path, macro_path = _write_macro(
        tmp_path, cut=cut, particle=particle, energy=energy
    )

    output_lh5 = tmp_path / f"output_{particle}.lh5"
    remage_run(
        str(macro_path),
        gdml_files=str(gdml_path),
        output=str(output_lh5),
        flat_output=True,
        overwrite_output=True,
    )

    det = _read_steps(output_lh5)
    num_steps = len(det.view_as("ak"))
    assert num_steps > 1, f"Expected more than one step recorded for {particle}"

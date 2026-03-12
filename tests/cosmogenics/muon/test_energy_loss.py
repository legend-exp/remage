from __future__ import annotations

import os

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
import pint
import pyg4ometry as pg4
import scipy as sp
from lgdo import lh5
from pygeomtools import RemageDetectorInfo
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

u = pint.get_application_registry()

macro = """
/RMG/Processes/HadronicPhysics {had_physics}
/RMG/Processes/LowEnergyEMPhysics {em_physics}
/RMG/Output/ActivateOutputScheme Track
/RMG/Geometry/RegisterDetectorsFromGDML Scintillator

/run/initialize

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Output/Track/StoreSinglePrecisionEnergy
/RMG/Output/Track/StoreSinglePrecisionPosition

/RMG/Output/Scintillator/StoreParticleVelocities true

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     -10 0 0
/gps/direction    1 0 0
/gps/particle     mu-
/gps/energy       {energy} GeV

/RMG/Processes/Stepping/KillSecondaries true

/run/beamOn {events}
"""


def geometry(mat_name: str):
    reg = pg4.geant4.Registry()
    matreg = LegendMaterialRegistry(reg, enable_optical=False)

    if mat_name == "lar":
        mat = matreg.liquidargon
    elif mat_name == "water":
        mat = matreg.water
    else:
        msg = f"unknown material {mat_name}"
        raise ValueError(msg)

    world_s = pg4.geant4.solid.Box("world", 20, 20, 20, registry=reg, lunit="cm")
    world_l = pg4.geant4.LogicalVolume(world_s, mat, "world", registry=reg)
    reg.setWorld(world_l)

    thin_slab_s = pg4.geant4.solid.Box(
        "thin_slab", 0.1, 20 - 0.001, 20 - 0.001, registry=reg, lunit="cm"
    )
    thin_slab_l = pg4.geant4.LogicalVolume(thin_slab_s, mat, "thin_slab", registry=reg)
    thin_slab_p = pg4.geant4.PhysicalVolume(
        [0, 0, 0],
        [10 - 0.05, 0, 0, "cm"],
        thin_slab_l,
        "detector",
        world_l,
        registry=reg,
    )

    thin_slab_p.set_pygeom_active_detector(RemageDetectorInfo("scintillator", 1, {}))

    return reg


def simulate(
    energy: float,
    material: str = "lar",
    had_physics: str = "Shielding",
    em_physics: str = "Livermore",
) -> str:
    output = (
        f"output-energy_loss-{energy:.0f}-{material}-{had_physics}-{em_physics}.lh5"
    )


    events = 100000 * int(os.environ.get("RMG_STATS_FACTOR", "1"))

    geom = geometry(material)

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "energy": energy,
            "events": events,
            "had_physics": had_physics,
            "em_physics": em_physics,
        },
        gdml_files=geom,
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


def simulate_and_plot(energy: float, material: str, had_physics: str, em_physics: str):

    fig, ax = plt.subplots()

    remage_output = simulate(
        energy, material=material, had_physics=had_physics, em_physics=em_physics
    )

    # read in track data
    tracks = lh5.read_as("tracks", remage_output, library="ak")
    mask = tracks["parent_trackid"] == 0

    # read in event data
    stp = lh5.read("stp/", remage_output)

    c = sp.constants.physical_constants["speed of light in vacuum"][0]
    m = sp.constants.physical_constants["muon mass energy equivalent in MeV"][0]

    v_in_m_per_s = stp["detector"]["v_post"].view_as("ak") * 1e9
    E_post = m / np.sqrt(1.0 - v_in_m_per_s**2 / c**2)
    E_kin_post = E_post - m

    E_lost = np.max(tracks[mask]["ekin"]) - ak.max(E_kin_post, axis=-1)
    distance = 20  # cm

    dEdx_sim = E_lost / distance

    densities = {
        "lar": 1.396,  # g/cm^3
        "water": 1.0,  # g/cm^3
    }

    c_minimal_ionization = 1.5  # MeV*cm^2/g
    dEdx_min_ionizing = c_minimal_ionization * densities[material]  # MeV/cm

    mean_dEdx = np.mean(dEdx_sim)
    median_dEdx = np.median(dEdx_sim)

    ax.hist(
        ak.ravel(dEdx_sim),
        bins=10 ** np.linspace(0, 5, 101),
        label="simulated",
        histtype="step",
    )
    ax.axvline(
        dEdx_min_ionizing,
        color="black",
        ls=":",
        label=f"minimal ionization = {dEdx_min_ionizing:.2f} MeV/cm",
    )
    ax.axvline(
        mean_dEdx, color="red", ls="--", label=f"mean dE/dx = {mean_dEdx:.2f} MeV/cm"
    )
    ax.axvline(
        median_dEdx,
        color="blue",
        ls="--",
        label=f"median dE/dx = {median_dEdx:.2f} MeV/cm",
    )

    ax.axvline(
        np.max(tracks[mask]["ekin"]) / distance,
        color="green",
        ls=":",
        label=f"upper limit from sim = {np.max(tracks[mask]['ekin']) / distance} MeV/cm",
    )
    ax.set_xscale("log")
    ax.set_yscale("log")

    ax.set_ylim(1, 1e5)

    ax.set_xlabel("energy loss dE/dx [MeV/cm]")
    ax.legend(loc="upper right")
    ax.text(
        3e2,
        1e3,
        "energy: \nmaterial: \nhadronic physics: \nEM physics: ",
        size=10,
        ha="left",
        va="top",
    )
    ax.text(
        1e5,
        1e2,
        1e3,
        f"{energy:.0f} GeV\n{material}\n{had_physics}\n{em_physics}",
        weight="bold",
        ha="right",
        va="top",
    )

    fig.savefig(
        f"energy_loss_{material}_{had_physics}_{em_physics}_{energy}GeV.output.png"
    )


def test_energy_loss():
    energies = [10.0, 100.0, 1000.0]
    materials = ["lar", "water"]
    had_physics_list = ["Shielding", "QGSP_BERT", "QGSP_BIC", "FTFP_BERT", "None"]
    em_physics_list = [
        "Option1",
        "Option4",
        "Penelope",
        "Livermore",
    ]

    for material in materials:
        for had_physics in had_physics_list:
            for em_physics in em_physics_list:
                for energy in energies:
                    simulate_and_plot(energy, material, had_physics, em_physics)

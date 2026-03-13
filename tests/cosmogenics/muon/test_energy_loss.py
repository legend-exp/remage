from __future__ import annotations

import os
from concurrent.futures import ProcessPoolExecutor, as_completed

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
import pint
import pyg4ometry as pg4
import scipy as sp
from dbetto import TextDB
from lgdo import lh5
from pygeomtools import RemageDetectorInfo
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

u = pint.get_application_registry()

FRACTION_OF_ENERGY_LOSS = 0.05

WORLD_WIDTH_CM = 50

ENERGY_LOSS = TextDB("./misc/")["energy_loss"]
DENSITIES = TextDB("./misc/")["densities"]

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
/gps/position     -{size} 0 0
/gps/direction    1 0 0
/gps/particle     mu-
/gps/energy       {energy} GeV

/RMG/Processes/Stepping/KillSecondaries true

/run/beamOn {events}
"""


def calculate_dEdx(remage_output: str, energy: float, material: str):
    # read in track data
    tracks = lh5.read_as("tracks", remage_output, library="ak")
    mask = tracks["parent_trackid"] == 0

    # read in event data
    try:
        stp = lh5.read("stp/", remage_output)
    except lh5.exceptions.LH5DecodeError:
        return np.array([0])

    c = sp.constants.physical_constants["speed of light in vacuum"][0]
    m = sp.constants.physical_constants["muon mass energy equivalent in MeV"][0]

    v_in_m_per_s = stp["detector"]["v_post"].view_as("ak") * 1e9
    E_post = m / np.sqrt(1.0 - v_in_m_per_s**2 / c**2)
    E_kin_post = E_post - m

    E_lost = np.max(tracks[mask]["ekin"]) - ak.max(E_kin_post, axis=-1)

    height = calculate_material_height(energy, material)

    return np.clip(
        np.array(E_lost / height), 0, 1e10
    )  # clip to reasonable values for better plotting


def calculate_material_height(energy, material):
    dEdx = np.interp(
        energy, ENERGY_LOSS["energy_points"][:, 0], ENERGY_LOSS["total"][material]
    )
    return energy * FRACTION_OF_ENERGY_LOSS / dEdx


def geometry(mat_name: str, energy: float = 1.0):
    reg = pg4.geant4.Registry()
    matreg = LegendMaterialRegistry(reg, enable_optical=False)

    if mat_name == "lar":
        mat = matreg.liquidargon
    elif mat_name == "water":
        mat = matreg.water
    else:
        msg = f"unknown material {mat_name}"
        raise ValueError(msg)

    world_height_cm = calculate_material_height(energy, mat_name)

    world_s = pg4.geant4.solid.Box(
        "world",
        world_height_cm,
        WORLD_WIDTH_CM,
        WORLD_WIDTH_CM,
        registry=reg,
        lunit="cm",
    )
    world_l = pg4.geant4.LogicalVolume(world_s, mat, "world", registry=reg)
    reg.setWorld(world_l)

    thin_slab_s = pg4.geant4.solid.Box(
        "thin_slab",
        world_height_cm * 0.001,
        WORLD_WIDTH_CM - 0.001,
        WORLD_WIDTH_CM - 0.001,
        registry=reg,
        lunit="cm",
    )
    thin_slab_l = pg4.geant4.LogicalVolume(thin_slab_s, mat, "thin_slab", registry=reg)
    thin_slab_p = pg4.geant4.PhysicalVolume(
        [0, 0, 0],
        [world_height_cm / 2 - world_height_cm * 0.001, 0, 0, "cm"],
        thin_slab_l,
        "detector",
        world_l,
        registry=reg,
    )

    thin_slab_p.set_pygeom_active_detector(RemageDetectorInfo("scintillator", 1, {}))

    return reg


def plot_energy_range(energies, materials, had_physics, em_physics, outfiles):

    for material in materials:
        dEdx_sims = {}
        for energy in energies:
            for had_physic in had_physics:
                for em_physic in em_physics:
                    remage_output = outfiles[(energy, material, had_physic, em_physic)]
                    dEdx_sims[energy] = calculate_dEdx(remage_output, energy, material)

        x = np.array(energies)
        y = np.array([np.mean(dEdx_sim) for dEdx_sim in dEdx_sims.values()])
        mask = y > 0
        x = x[mask]
        y = y[mask]
        y_unc = np.array(
            [
                np.std(dEdx_sim) / np.sqrt(len(dEdx_sim))
                for dEdx_sim in dEdx_sims.values()
            ]
        )

        x_exp = np.array(ENERGY_LOSS["energy_points"])
        y_exp = np.array(ENERGY_LOSS["total"][material]) * DENSITIES[material]

        fig, ax = plt.subplots()
        ax.errorbar(x, y, yerr=y_unc, fmt="o", label="simulation")
        ax.plot(x_exp, y_exp, label="expected")
        ax.set_xscale("log")
        ax.set_xlabel("muon energy [GeV]")
        ax.set_ylabel("mean energy loss dE/dx [MeV/cm]")
        ax.legend()
        ax.set_title(
            f"Energy loss of muons in {material}",
            size=8,
        )
        fig.savefig(f"energy_loss_{material}_energy_range.output.png")


def _simulate_case(
    case: tuple[float, str, str, str], max_threads: int = 1
) -> tuple[tuple[float, str, str, str], str]:

    energy, material, had_physics, em_physics = case
    if energy > 1:
        output = (
            f"output-energy_loss-{energy:.0f}-{material}-{had_physics}-{em_physics}.lh5"
        )
    else:
        output = (
            f"output-energy_loss-{energy:.2f}-{material}-{had_physics}-{em_physics}.lh5"
        )

    events = 100 * int(os.environ.get("RMG_STATS_FACTOR", "1"))

    geom = geometry(material, energy)

    world_height_cm = calculate_material_height(energy, material)

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "energy": energy,
            "events": events,
            "had_physics": had_physics,
            "em_physics": em_physics,
            "size": world_height_cm / 2,
        },
        gdml_files=geom,
        output=output,
        overwrite_output=True,
        log_level="summary",
        threads=max_threads,
        merge_output_files=True,
    )

    return (energy, material, had_physics, em_physics), output


def test_energy_loss():
    energies = np.array(ENERGY_LOSS["energy_points"])[:, 0][10::2] / 1e3
    materials = ["lar", "water"]
    had_physics_list = ["Shielding"]
    em_physics_list = ["Livermore"]

    cases = [
        (energy, material, had_physics, em_physics)
        for material in materials
        for had_physics in had_physics_list
        for em_physics in em_physics_list
        for energy in energies
    ]

    max_workers = min(len(cases), os.cpu_count() // 2)
    max_threads = os.cpu_count() // 2 // max_workers
    max_threads = max(1, max_threads)

    outfiles = {}
    if max_workers == 1:
        for case in cases:
            key, output = _simulate_case(case, max_threads=max_threads)
            outfiles[key] = output
    else:
        with ProcessPoolExecutor(max_workers=max_workers) as ex:
            futures = [
                ex.submit(_simulate_case, case, max_threads=max_threads)
                for case in cases
            ]
            for fut in as_completed(futures):
                key, output = fut.result()
                outfiles[key] = output

    plot_energy_range(energies, materials, had_physics_list, em_physics_list, outfiles)

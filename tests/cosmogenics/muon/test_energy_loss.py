from __future__ import annotations

import os

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
import pint
import pyg4ometry as pg4
from lgdo import lh5
from pygeomtools import RemageDetectorInfo
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

u = pint.get_application_registry()

macro = """
/RMG/Processes/HadronicPhysics Shielding
/RMG/Output/ActivateOutputScheme Track
/RMG/Geometry/RegisterDetectorsFromGDML Scintillator

/run/initialize

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Output/Track/StoreSinglePrecisionEnergy
/RMG/Output/Track/StoreSinglePrecisionPosition

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
    else:
        msg = f"unknown material {mat_name}"
        raise ValueError(msg)

    world_s = pg4.geant4.solid.Box("world", 20, 20, 20, registry=reg, lunit="cm")
    world_l = pg4.geant4.LogicalVolume(world_s, mat, "world", registry=reg)
    reg.setWorld(world_l)

    thin_slab_s = pg4.geant4.solid.Box(
        "thin_slab", 20, 20, 0.1, registry=reg, lunit="cm"
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


def simulate(energy: int, material: str = "lar"):
    output = f"output-energy_loss-{energy}.lh5"

    events = 100 * int(os.environ.get("RMG_STATS_FACTOR", "1"))

    remage_run(
        macro.split("\n"),
        macro_substitutions={"energy": energy, "events": events},
        gdml_files=geometry(material),
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


def simulate_and_plot(energies: list[int], material: str):
    results = {}

    for energy in energies:
        results[energy] = {}
        fig, (ax1, ax2) = plt.subplots(1, 2)

        remage_output = simulate(energy, material=material)

        # read in track data
        tracks = lh5.read_as("tracks", remage_output, library="ak")

        # read in event data
        sct_output = lh5.read_as("stp/thin_slab", remage_output, library="ak")

        # group-by event id
        trk = ak.unflatten(tracks, ak.run_lengths(tracks.evtid))

        # select optical photons.
        ptrk = trk[(trk.particle == -22)]

        num_phot = ak.num(ptrk.particle, axis=1)

        h = hist.new.Reg(200, 0, 5000).Double().fill(num_phot)
        h.plot(
            ax=ax1,
            yerr=False,
            label=rf"energy = {energy} keV",
        )
        results[particle][energy] = np.mean(num_phot)

        wavelengths = ak.flatten(1239 / ptrk.ekin / 1e6)
        bounds = (100, 200) if scintillate else (0, 800)
        h = hist.new.Reg(100, *bounds).Double().fill(wavelengths)
        h.plot(
            ax=ax2,
            yerr=False,
            label=rf"energy = {energy} keV",
            density=True,
        )

    fig.suptitle(f"{label} light yield for {particle}")
    ax1.set_xlabel(f"emitted {label} photon density [a.u.]")
    ax1.legend()
    ax2.set_xlabel(f"{label} photon wavelength [nm]")
    fig.savefig(f"{label.lower()}-{particle}.output.png")

    # plot linear dependency.
    endpoint = max(max(r) for r in results.values()) * 1.1

    fig, ax = plt.subplots()
    x = np.array([0, endpoint])
    for particle, _, expectation in items:
        if scintillate:
            ax.plot(x, x * expectation, color="black", linestyle="--", linewidth=1)
        ax.scatter(
            results[particle].keys(),
            results[particle].values(),
            label=particle,
            marker="x",
            zorder=100,
        )

    ax.set_title(f"{label} light yield")
    ax.set_ylabel(f"number of emitted {label} photons")
    ax.set_xlabel("particle energy [keV]")
    ax.set_xlim([0, endpoint])
    ax.legend()
    fig.savefig(f"{label.lower()}-yield.output.png")


def test_energy_loss():
    energies = [1, 10, 100, 500, 1000]
    materials = ["lar", "water"]

    for material in materials:
        simulate_and_plot(energies, material)

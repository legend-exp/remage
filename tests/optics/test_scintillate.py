from __future__ import annotations

import os

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
import pint
import pyg4ometry as pg4
import pygeomoptics
from lgdo import lh5
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

u = pint.get_application_registry()

macro = """
/RMG/Processes/OpticalPhysics
/RMG/Output/ActivateOutputScheme Track

/run/initialize

/RMG/Output/Track/StoreOpticalPhotons
/RMG/Output/Track/StoreSinglePrecisionEnergy
/RMG/Output/Track/StoreSinglePrecisionPosition

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     0 0 0

# a bit ugly. set an ion and then reset to the desired particle.
/gps/particle     ion
/gps/ion          6 12
/gps/particle     {particle}

/gps/energy       {energy} keV
/gps/ang/type     iso

# disable the respective other optical photon process.
/process/inactivate {inactive_proc} all

/run/beamOn {events}
"""


def geometry_emission(mat_name: str):
    reg = pg4.geant4.Registry()
    matreg = LegendMaterialRegistry(reg, enable_optical=False)

    if mat_name == "lar":
        mat = matreg.liquidargon
        pygeomoptics.lar.pyg4_lar_attach_rindex(mat, reg)
        pygeomoptics.lar.pyg4_lar_attach_scintillation(mat, reg)
    elif mat_name == "water":
        mat = matreg.water
        pygeomoptics.water.pyg4_water_attach_rindex(mat, mat)
    else:
        msg = f"unknown material {mat_name}"
        raise ValueError(msg)

    world_s = pg4.geant4.solid.Orb("world", 20, registry=reg, lunit="cm")
    world_l = pg4.geant4.LogicalVolume(world_s, mat, "world", registry=reg)
    reg.setWorld(world_l)

    return reg


def simulate(particle: str, energy: int, scintillate: bool):
    output = f"output-{'scintillation' if scintillate else 'cerenkov'}-{particle}-{energy}.lh5"

    events = 100 * int(os.environ.get("RMG_STATS_FACTOR", "1"))

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "particle": particle,
            "energy": energy,
            "events": events,
            "inactive_proc": "Cerenkov" if scintillate else "Scintillation",
        },
        gdml_files=geometry_emission("lar" if scintillate else "water"),
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


def simulate_and_plot(items, scintillate: bool, label: str):
    results = {}

    for particle, energies, _ in items:
        results[particle] = {}
        fig, (ax1, ax2) = plt.subplots(1, 2)

        for energy in energies:
            remage_output = simulate(particle, energy, scintillate=scintillate)

            # read in track data
            tracks = lh5.read_as("tracks", remage_output, library="ak")

            # read in dictionary with process ids, to filter later
            processes = lh5.read("processes", remage_output)
            assert scintillate == ("Scintillation" in processes)
            assert scintillate != ("Cerenkov" in processes)

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
            h = hist.new.Reg(100, 0, 800).Double().fill(wavelengths)
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


def test_scintillate():
    items_scintillate = [
        ("e-", [10, 50, 100], 25),
        ("alpha", [10, 50, 100], 21.8),
        ("ion", [10, 50, 100], 9.3),
    ]

    simulate_and_plot(items_scintillate, scintillate=True, label="scintillation")


def test_cerenkov():
    items_scintillate = [
        ("e-", [500, 1000, 2000, 5000, 10000, 20000, 30000], None),
    ]

    simulate_and_plot(items_scintillate, scintillate=False, label="Cerenkov")

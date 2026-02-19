from __future__ import annotations

import os

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
import pint
import pyg4ometry as pg4
import pygeomoptics
import pytest
from _geometry import _add_dummy_sipm_surface
from lgdo import lh5
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run
from scipy.optimize import curve_fit

u = pint.get_application_registry()

macro = """
/RMG/Processes/OpticalPhysics

/RMG/Geometry/RegisterDetector Optical detector 001

/run/initialize

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     0 0 {z} mm
/gps/particle     opticalphoton
/gps/energy       9.68 eV
#/gps/ang/type     iso
/gps/direction    0 0 1

/run/beamOn {events}
"""

chamber_length_mm = 200


def geometry_attenuation(setup):
    reg = pg4.geant4.Registry()
    matreg = LegendMaterialRegistry(reg, enable_optical=False)

    world_s = pg4.geant4.solid.Box("world", 1000, 1000, 1000, registry=reg, lunit="mm")
    world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
    reg.setWorld(world_l)

    mat = matreg.liquidargon
    pygeomoptics.lar.pyg4_lar_attach_rindex(mat, reg)
    pygeomoptics.lar.pyg4_lar_attach_attenuation(
        mat, reg, lar_temperature=89 * u.K, **setup
    )

    chamber_length_mm = 200
    chamber_s = pg4.geant4.solid.Tubs(
        "chamber", 0, 100, chamber_length_mm, 0, 2 * np.pi, reg, "mm"
    )
    chamber_l = pg4.geant4.LogicalVolume(chamber_s, mat, "chamber", registry=reg)
    pg4.geant4.PhysicalVolume(
        [0, 0, 0], [0, 0, 0], chamber_l, "chamber", world_l, registry=reg
    )

    det_thickness = 0.1
    det_s = pg4.geant4.solid.Tubs(
        "detector", 0, 5, det_thickness, 0, 2 * np.pi, reg, "mm"
    )
    det_l = pg4.geant4.LogicalVolume(
        det_s, matreg.metal_silicon, "detector", registry=reg
    )
    pg4.geant4.PhysicalVolume(
        [0, 0, 0],
        [0, 0, chamber_length_mm / 2 - det_thickness, "mm"],
        det_l,
        "detector",
        chamber_l,
        registry=reg,
    )

    _add_dummy_sipm_surface(0, 1, reg, det_l)

    return reg


def simulate(distance_mm: float, label, setup):
    label_file = label.replace(" ", "-").lower()
    output = f"output-attenuation-{label_file}-{distance_mm:.2f}.lh5"

    events = 5000 * int(os.environ.get("RMG_STATS_FACTOR", "1"))

    z = chamber_length_mm / 2 - distance_mm

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "events": events,
            "z": z,
        },
        gdml_files=geometry_attenuation(setup),
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


@pytest.mark.parametrize("attenuation", [30, 60])
def test_attenuation(attenuation: float):
    fig, ax = plt.subplots()

    att = attenuation * u.cm
    items = [
        (
            "Rayleigh only",
            {"rayleigh_enabled_or_length": att, "absorption_enabled_or_length": False},
        ),
        (
            "absorption only",
            {"rayleigh_enabled_or_length": False, "absorption_enabled_or_length": att},
        ),
        ("both", {"attenuation_method_or_length": att}),
    ]

    for label, setup in items:
        x = []
        y = []

        for distance in np.arange(10, 200, 10):
            remage_output = simulate(distance, label, setup)

            stps = lh5.read_as("stp/det001", remage_output, library="ak")
            x.append(distance)
            y.append(ak.num(stps.wavelength, axis=0))

        def exp_fit(x, a, b):
            return a * np.exp(-b * x)

        x = np.array(x)
        y = np.array(y)
        popt, _pcov = curve_fit(exp_fit, x, y, p0=(10000, 1 / 300))

        x_exp = np.linspace(0, 190)
        ax.plot(x_exp, exp_fit(x_exp, *popt), linestyle="--")

        display_label = f"{label} → $\\Lambda = {1 / popt[1] / 10:.1f}\\mathrm{{ cm}}$"
        ax.scatter(x, y, marker="x", zorder=1, label=display_label)

    ax.legend()
    ax.set_title("attenuation")
    ax.set_ylabel("detected photons")
    ax.set_xlabel(r"distance to detector")
    fig.savefig(f"photon-attenuation-{attenuation}.output.png")

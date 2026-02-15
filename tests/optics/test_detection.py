from __future__ import annotations

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
import pint
import pyg4ometry as pg4
import pygeomoptics
from lgdo import lh5
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

u = pint.get_application_registry()

n_events = 3000
macro = """
/RMG/Processes/OpticalPhysics

/RMG/Geometry/RegisterDetector Optical detector 001

/run/initialize

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     0 0 18 cm
/gps/particle     opticalphoton
/gps/energy       6 eV
/gps/direction    0 0 -1

/run/beamOn {events}
"""


def _add_dummy_sipm_surface(r, e, reg, det_l):
    det_surf = pg4.geant4.solid.OpticalSurface(
        "det_surf",
        finish="ground",
        model="unified",
        surf_type="dielectric_metal",
        value=0.05,
        registry=reg,
    )

    wvl = np.array([100, 800]) * u.nm
    eff = np.array([e, e])
    refl = np.array([r, r])

    with u.context("sp"):
        det_surf.addVecPropertyPint("EFFICIENCY", wvl.to("eV"), eff)
        det_surf.addVecPropertyPint("REFLECTIVITY", wvl.to("eV"), refl)

    pg4.geant4.SkinSurface("det_surf", det_l, det_surf, reg)


def geometry_detection(r: float, e: float):
    reg = pg4.geant4.Registry()
    matreg = LegendMaterialRegistry(reg, enable_optical=False)

    mat = matreg.liquidargon
    pygeomoptics.lar.pyg4_lar_attach_rindex(mat, reg)

    world_s = pg4.geant4.solid.Orb("world", 20, registry=reg, lunit="cm")
    world_l = pg4.geant4.LogicalVolume(world_s, mat, "world", registry=reg)
    reg.setWorld(world_l)

    det_s = pg4.geant4.solid.Orb("detector", 15, registry=reg, lunit="cm")
    det_l = pg4.geant4.LogicalVolume(
        det_s, matreg.metal_silicon, "detector", registry=reg
    )
    pg4.geant4.PhysicalVolume(
        [0, 0, 0], [0, 0, 0], det_l, "detector", world_l, registry=reg
    )

    _add_dummy_sipm_surface(r, e, reg, det_l)

    return reg


def simulate(r: float, e: float):
    output = f"output-detection-{e:.2f}-{r:.2f}.lh5"

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "events": n_events,
        },
        gdml_files=geometry_detection(r, e),
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


def test_detection():
    fig, ax = plt.subplots()

    x = []
    y = []
    for e in (0.1, 0.5, 1):
        for r in (0.1, 0.5, 0.9):
            remage_output = simulate(r, e)

            stps = lh5.read_as("stp/det001", remage_output, library="ak")
            x.append((1 - r) * e)
            y.append(ak.num(stps.wavelength, axis=0) / n_events)

    ax.plot([0, 1], [0, 1], color="black", linestyle="--", linewidth=1)
    ax.scatter(x, y, marker="x", zorder=1)

    ax.set_title("sensitive surfaces")
    ax.set_ylabel("fraction of detected photons")
    ax.set_xlabel(r"$(1-R) \cdot \epsilon$")
    fig.savefig("photon-detection.output.png")

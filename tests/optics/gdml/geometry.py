from __future__ import annotations

import numpy as np
import pint
import pyg4ometry as pg4
import pygeomoptics
from pygeomtools.materials import LegendMaterialRegistry

u = pint.get_application_registry()

# EMISSION TESTS.
for mat_name in ("lar", "water"):
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

    w = pg4.gdml.Writer()
    w.addDetector(reg)
    w.write(f"geometry-emission-{mat_name}.gdml")

# DETECTION TESTS
for e in (0.1, 0.5, 1):
    for r in (0.1, 0.5, 0.9):
        reg = pg4.geant4.Registry()
        matreg = LegendMaterialRegistry(reg, enable_optical=False)

        mat = matreg.liquidargon
        pygeomoptics.lar.pyg4_lar_attach_rindex(mat, reg)

        world_s = pg4.geant4.solid.Orb("world", 20, registry=reg, lunit="cm")
        world_l = pg4.geant4.LogicalVolume(world_s, mat, "world", registry=reg)
        reg.setWorld(world_l)

        det_mat = matreg.metal_silicon
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

        det_s = pg4.geant4.solid.Orb("detector", 15, registry=reg, lunit="cm")
        det_l = pg4.geant4.LogicalVolume(det_s, det_mat, "detector", registry=reg)
        pg4.geant4.PhysicalVolume(
            [0, 0, 0], [0, 0, 0], det_l, "detector", world_l, registry=reg
        )

        pg4.geant4.SkinSurface("det_surf", det_l, det_surf, reg)

        w = pg4.gdml.Writer()
        w.addDetector(reg)
        w.write(f"geometry-detection-{e:.2f}-{r:.2f}.gdml")

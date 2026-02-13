from __future__ import annotations

import pyg4ometry as pg4
import pygeomoptics
from pygeomtools.materials import LegendMaterialRegistry

# EMISSION TESTS.
for mat_name in ("lar", "water"):
    reg = pg4.geant4.Registry()

    if mat_name == "lar":
        mat = LegendMaterialRegistry(reg, enable_optical=False).liquidargon
        pygeomoptics.lar.pyg4_lar_attach_rindex(mat, reg)
        pygeomoptics.lar.pyg4_lar_attach_scintillation(mat, reg)
    elif mat_name == "water":
        mat = LegendMaterialRegistry(reg, enable_optical=False).water
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

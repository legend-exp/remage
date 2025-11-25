from __future__ import annotations

import numpy as np
import pyg4ometry as pg4
import pygeomhpges as hpges
from pygeomtools import write_pygeom

reg = pg4.geant4.Registry()

# create a world volume
world_s = pg4.geant4.solid.Orb("world", 200, registry=reg, lunit="mm")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
reg.setWorld(world_l)


# lar balloon
lar_s = pg4.geant4.solid.Orb("LAr_s", 100, registry=reg, lunit="mm")
lar_l = pg4.geant4.LogicalVolume(lar_s, "G4_lAr", "LAr_l", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "LAr", world_l, registry=reg)
lar_l.pygeom_color_rgba = (1, 0, 1, 0.1)


# let's make a germanium balloon
natge = hpges.materials.make_natural_germanium(reg)
ge_s = pg4.geant4.solid.Tubs(
    "germanium",
    pRMin=0,
    pRMax=40,
    pDz=40,
    pSPhi=0,
    pDPhi=2 * np.pi,
    registry=reg,
    lunit="mm",
)
ge_l = pg4.geant4.LogicalVolume(ge_s, natge, "germanium", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], ge_l, "germanium", lar_l, registry=reg)
ge_l.pygeom_color_rgba = (0, 1, 1, 0.9)


# finally create a small radioactive source
source_s = pg4.geant4.solid.Tubs("Source_s", 0, 4, 1, 0, 2 * np.pi, registry=reg)
source_l = pg4.geant4.LogicalVolume(source_s, "G4_Fe", "Source_L", registry=reg)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 5, "cm"], source_l, "Source", lar_l, registry=reg
)

write_pygeom(reg, "gdml/geometry.gdml")

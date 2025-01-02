from __future__ import annotations

import legendhpges as hpges
import pyg4ometry as pg4

reg = pg4.geant4.Registry()

# create a world volume
world_s = pg4.geant4.solid.Orb("world", 20, registry=reg, lunit="cm")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
reg.setWorld(world_l)

# let's make a germanium balloon
natge = hpges.materials.make_natural_germanium(reg)

ge_s = pg4.geant4.solid.Orb("germanium", 15, registry=reg, lunit="cm")
ge_l = pg4.geant4.LogicalVolume(ge_s, natge, "germanium", registry=reg)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], ge_l, "germanium", world_l, registry=reg
)

w = pg4.gdml.Writer()
w.addDetector(reg)
w.write("geometry.gdml")

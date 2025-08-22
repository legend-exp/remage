from __future__ import annotations

import pyg4ometry as pg4

reg = pg4.geant4.Registry()

world_s = pg4.geant4.solid.Orb("world", 20, registry=reg, lunit="cm")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
reg.setWorld(world_l)

ge_s = pg4.geant4.solid.Orb("vacuum", 15, registry=reg, lunit="cm")
ge_l = pg4.geant4.LogicalVolume(ge_s, "G4_Galactic", "vacuum", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], ge_l, "vacuum", world_l, registry=reg)

w = pg4.gdml.Writer()
w.addDetector(reg)
w.write("geometry.gdml")

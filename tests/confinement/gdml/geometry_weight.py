from __future__ import annotations

from math import pi

from pyg4ometry import gdml, geant4

reg = geant4.Registry()

world_side = 10  # m

# world volume (air)
world_s = geant4.solid.Box("World", world_side, world_side, world_side, reg, lunit="m")
world_l = geant4.LogicalVolume(world_s, "G4_Pb", "World", reg)
reg.setWorld(world_l)

# 2 boxes with one smaller
box_s = geant4.solid.Box("Box", 1, 1, 1, reg, lunit="m")
box1_l = geant4.LogicalVolume(box_s, "G4_Pb", "Box1", reg)
geant4.PhysicalVolume(
    [pi / 4, -pi / 4, pi / 4], [2, 0, 0, "m"], box1_l, "Box1", world_l, reg
)

box2_l = geant4.LogicalVolume(box_s, "G4_Be", "Box2", reg)
geant4.PhysicalVolume(
    [pi / 4, -pi / 4, pi / 4], [-2, 0, 0, "m"], box2_l, "Box2", world_l, reg
)

world_l.checkOverlaps(recursive=True)

w = gdml.Writer()
w.addDetector(reg)
w.write("geometry_weight.gdml")

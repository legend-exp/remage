from __future__ import annotations

from math import pi

from pyg4ometry import gdml, geant4

reg = geant4.Registry()

world_side = 10  # m

# world volume (air)
world_s = geant4.solid.Box("World", world_side, world_side, world_side, reg, lunit="m")
world_l = geant4.LogicalVolume(world_s, "G4_Pb", "World", reg)
reg.setWorld(world_l)

# solids that can be sampled natively
#####################################

# box
box_s = geant4.solid.Box("Box", 1, 1, 1, reg, lunit="m")
box_l = geant4.LogicalVolume(box_s, "G4_Pb", "Box", reg)
geant4.PhysicalVolume(
    [pi / 4, -pi / 4, pi / 4], [2, 0, 0, "m"], box_l, "Box", world_l, reg
)

# orb
orb_s = geant4.solid.Orb("Orb", 0.5, reg, lunit="m")
orb_l = geant4.LogicalVolume(orb_s, "G4_Pb", "Orb", reg)
geant4.PhysicalVolume([0, 0, 0], [0, 2, 0, "m"], orb_l, "Orb", world_l, reg)

# spherical thing
sphere_s = geant4.solid.Sphere(
    "Sphere", 0.2, 0.4, pi / 3, 2 * pi / 3, pi / 6, pi / 2, reg, lunit="m"
)
sphere_l = geant4.LogicalVolume(sphere_s, "G4_Pb", "Sphere", reg)
geant4.PhysicalVolume(
    [pi / 4, pi / 4, -pi / 4], [0, 0, 2, "m"], sphere_l, "Sphere", world_l, reg
)

# tubs
tubs_s = geant4.solid.Tubs("Tubs", 0.2, 0.5, 1.2, pi / 3, 4 * pi / 3, reg, lunit="m")
tubs_l = geant4.LogicalVolume(tubs_s, "G4_Pb", "Tubs", reg)
geant4.PhysicalVolume(
    [-pi / 4, pi / 4, pi / 4], [-2, 0, 0, "m"], tubs_l, "Tubs", world_l, reg
)

# complex solids
################

# box with spherical hole (subtraction)
box2_s = geant4.solid.Box("Box2", 1, 1, 1, reg, lunit="m")
hole_s = geant4.solid.Orb("Hole", 0.6, reg, lunit="m")
boxwhole_s = geant4.solid.Subtraction(
    "BoxWithHole", box2_s, hole_s, [[0, 0, 0], [0, 0, 0]], reg
)
boxwhole_l = geant4.LogicalVolume(boxwhole_s, "G4_Pb", "BoxWithHole", reg)
geant4.PhysicalVolume(
    [pi / 4, -pi / 4, pi / 4], [0, 0, 0, "m"], boxwhole_l, "BoxWithHole", world_l, reg
)

# box and sphere (union)
boxnorb_s = geant4.solid.Union("BoxAndOrb", box2_s, hole_s, [[0, 0, 0], [0, 0, 0]], reg)
boxnorb_l = geant4.LogicalVolume(boxnorb_s, "G4_Pb", "BoxAndOrb", reg)
geant4.PhysicalVolume(
    [pi / 4, -pi / 4, pi / 4], [2, 2, 0, "m"], boxnorb_l, "BoxAndOrb", world_l, reg
)

# above box with hole inside a mother orb
boxnorb2_s = geant4.solid.Union(
    "BoxAndOrb2", box2_s, hole_s, [[0, 0, 0], [0, 0, 0]], reg
)
boxnorb2_l = geant4.LogicalVolume(boxnorb2_s, "G4_Pb", "BoxAndOrb2", reg)
orb_s = geant4.solid.Orb("MotherOrb", 1, reg, lunit="m")
orb_l = geant4.LogicalVolume(orb_s, "G4_Pb", "MotherOrb", reg)
geant4.PhysicalVolume([0, 0, 0], [0, 0, 0, "m"], boxnorb2_l, "BoxAndOrb2", orb_l, reg)
geant4.PhysicalVolume([0, 0, 0], [2, 2, 2, "m"], orb_l, "MotherOrb", world_l, reg)

# polycone
poly_s = geant4.solid.Polycone(
    "Polycone", 0, 2 * pi, [-0.5, 0.5], [0, 0], [0.5, 0.3], reg, lunit="m"
)
poly_l = geant4.LogicalVolume(poly_s, "G4_Pb", "Polycone", reg)
geant4.PhysicalVolume([0, 0, 0], [0, 0, -2, "m"], poly_l, "Polycone", world_l, reg)

world_l.checkOverlaps(recursive=True)

w = gdml.Writer()
w.addDetector(reg)
w.write("geometry.gdml")

# v = vis.VtkViewer()
# v.addLogicalVolume(reg.getWorldVolume())
# v.setCameraFocusPosition(focalPoint=[0,0,0], position=[1,0,0])
# v.view()

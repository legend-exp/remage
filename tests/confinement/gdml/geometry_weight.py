from __future__ import annotations

from math import pi

from pyg4ometry import gdml, geant4

reg = geant4.Registry()

# dummy isotopes/elements with numbers that are easy to calculate.
iso1 = geant4.Isotope("isotope1", 32, 76, 76, reg)
iso2 = geant4.Isotope("isotope2", 32, 75, 75, reg)

el1 = geant4.ElementIsotopeMixture("element1", "X", 2, reg)
el1.add_isotope(iso1, 0.5)
el1.add_isotope(iso2, 0.5)

el2 = geant4.ElementIsotopeMixture("element2", "X", 2, reg)
el2.add_isotope(iso1, 0.25)
el2.add_isotope(iso2, 0.75)

mat1 = geant4.MaterialCompound("mat1", 10, 1, reg)
mat1.add_element_massfraction(el1, 1)

mat2 = geant4.MaterialCompound("mat2", 5, 1, reg)
mat2.add_element_massfraction(el2, 1)

# world volume (air)
world_side = 10  # m
world_s = geant4.solid.Box("World", world_side, world_side, world_side, reg, lunit="m")
world_l = geant4.LogicalVolume(world_s, "G4_Pb", "World", reg)
reg.setWorld(world_l)

# 2 boxes with one smaller
box_s = geant4.solid.Box("Box", 1, 1, 1, reg, lunit="m")
box1_l = geant4.LogicalVolume(box_s, mat1, "Box1", reg)
geant4.PhysicalVolume(
    [pi / 4, -pi / 4, pi / 4], [2, 0, 0, "m"], box1_l, "Box1", world_l, reg
)

box2_l = geant4.LogicalVolume(box_s, mat2, "Box2", reg)
geant4.PhysicalVolume(
    [pi / 4, -pi / 4, pi / 4], [-2, 0, 0, "m"], box2_l, "Box2", world_l, reg
)

world_l.checkOverlaps(recursive=True)

w = gdml.Writer()
w.addDetector(reg)
w.write("geometry_weight.gdml")

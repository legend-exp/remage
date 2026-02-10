from __future__ import annotations

from math import pi

import pyg4ometry as pg4
from pygeomtools import write_pygeom

reg = pg4.geant4.Registry()

Hydrogen = pg4.geant4.ElementSimple(
    name="Hydrogen", symbol="H", Z=1, A=1.008, registry=reg
)
Gadolinium = pg4.geant4.ElementSimple(
    name="Gadolinium", symbol="Gd", Z=64, A=157.25, registry=reg
)

# Just make some moderator material that contains Gd and is PMMA like. Does not have to be exact.
moderator_material = pg4.geant4.MaterialCompound(
    name="moderator_mat",
    density=1.2,
    number_of_components=2,
    registry=reg,
)
moderator_material.add_element_massfraction(Hydrogen, 0.97)
moderator_material.add_element_massfraction(Gadolinium, 0.03)

world_s = pg4.geant4.solid.Tubs(
    "world", 0, 2000, 2000, 0, 2 * pi, registry=reg, lunit="mm"
)
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
reg.setWorld(world_l)

water_solid = pg4.geant4.solid.Tubs(
    "water", 0, 1900, 1900, 0, 2 * pi, registry=reg, lunit="mm"
)
water_l = pg4.geant4.LogicalVolume(water_solid, "G4_WATER", "water", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], water_l, "water", world_l, registry=reg)

steel_solid = pg4.geant4.solid.Tubs(
    "steel", 0, 1600, 1600, 0, 2 * pi, registry=reg, lunit="mm"
)
steel_l = pg4.geant4.LogicalVolume(
    steel_solid, "G4_STAINLESS-STEEL", "steel", registry=reg
)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], steel_l, "steel", water_l, registry=reg)

argon_solid = pg4.geant4.solid.Tubs(
    "argon", 0, 1400, 1400, 0, 2 * pi, registry=reg, lunit="mm"
)
argon_l = pg4.geant4.LogicalVolume(argon_solid, "G4_lAr", "argon", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], argon_l, "argon", steel_l, registry=reg)

moderator_solid = pg4.geant4.solid.Box(
    "moderator", 20, 1400, 900, registry=reg, lunit="mm"
)
moderator_l = pg4.geant4.LogicalVolume(
    moderator_solid, moderator_material, "moderator", registry=reg
)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [100, 0, 0], moderator_l, "moderator", argon_l, registry=reg
)

write_pygeom(reg, "gdml/geometry.gdml")

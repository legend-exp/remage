from __future__ import annotations

from math import pi

import pyg4ometry as pg4
from pygeomtools import write_pygeom

# First create the basic geometry for the primary check
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
    [0, 0, 0], [300, 0, 0], moderator_l, "moderator", argon_l, registry=reg
)

write_pygeom(reg, "gdml/geometry.gdml")

# Now create smaller geometries consisting of only one material. This is for testing the capture distributions.

reg2 = pg4.geant4.Registry()

world_s = pg4.geant4.solid.Orb("world", 200, registry=reg2, lunit="m")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg2)
reg2.setWorld(world_l)

mat_s = pg4.geant4.solid.Orb("material", 199.99, registry=reg2, lunit="m")
mat_l = pg4.geant4.LogicalVolume(mat_s, "G4_Gd", "material", registry=reg2)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], mat_l, "material", world_l, registry=reg2
)
write_pygeom(reg2, "gdml/nist_gd_world.gdml")


reg2 = pg4.geant4.Registry()

world_s = pg4.geant4.solid.Orb("world", 200, registry=reg2, lunit="m")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg2)
reg2.setWorld(world_l)

mat_s = pg4.geant4.solid.Orb("material", 199.99, registry=reg2, lunit="m")
mat_l = pg4.geant4.LogicalVolume(mat_s, "G4_STAINLESS-STEEL", "material", registry=reg2)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], mat_l, "material", world_l, registry=reg2
)
write_pygeom(reg2, "gdml/nist_steel_world.gdml")

reg2 = pg4.geant4.Registry()

world_s = pg4.geant4.solid.Orb("world", 2000, registry=reg2, lunit="m")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg2)
reg2.setWorld(world_l)

argon_40_element = pg4.geant4.ElementIsotopeMixture(
    name="Argon", symbol="Ar", n_comp=1, registry=reg2
)
argon_40 = pg4.geant4.Isotope(name="Argon40", Z=18, N=40, a=39.97, registry=reg2)
argon_40_element.add_isotope(argon_40, 1.0)

argon_40_material = pg4.geant4.Material(
    name="LiquidArgon40",
    density=1.396,  # g/cm3
    number_of_components=1,
    state="liquid",
    temperature=88.8,  # K
    pressure=1.0 * 1e5,  # pascal
    registry=reg2,
)
argon_40_material.add_element_massfraction(argon_40_element, 1.0)

mat_s = pg4.geant4.solid.Orb("material", 1999.99, registry=reg2, lunit="m")
mat_l = pg4.geant4.LogicalVolume(mat_s, argon_40_material, "material", registry=reg2)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], mat_l, "material", world_l, registry=reg2
)
write_pygeom(reg2, "gdml/largon_40_world.gdml")

reg2 = pg4.geant4.Registry()

world_s = pg4.geant4.solid.Orb("world", 2000, registry=reg2, lunit="m")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg2)
reg2.setWorld(world_l)

argon_38_element = pg4.geant4.ElementIsotopeMixture(
    name="Argon", symbol="Ar", n_comp=1, registry=reg2
)
argon_38 = pg4.geant4.Isotope(name="Argon38", Z=18, N=38, a=37.97, registry=reg2)
argon_38_element.add_isotope(argon_38, 1.0)

argon_38_material = pg4.geant4.Material(
    name="LiquidArgon38",
    density=1.396,  # g/cm3
    number_of_components=1,
    state="liquid",
    temperature=88.8,  # K
    pressure=1.0 * 1e5,  # pascal
    registry=reg2,
)
argon_38_material.add_element_massfraction(argon_38_element, 1.0)

mat_s = pg4.geant4.solid.Orb("material", 1999.99, registry=reg2, lunit="m")
mat_l = pg4.geant4.LogicalVolume(mat_s, argon_38_material, "material", registry=reg2)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], mat_l, "material", world_l, registry=reg2
)
write_pygeom(reg2, "gdml/largon_38_world.gdml")

reg2 = pg4.geant4.Registry()

world_s = pg4.geant4.solid.Orb("world", 2000, registry=reg2, lunit="m")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg2)
reg2.setWorld(world_l)

argon_36_element = pg4.geant4.ElementIsotopeMixture(
    name="Argon", symbol="Ar", n_comp=1, registry=reg2
)
argon_36 = pg4.geant4.Isotope(name="Argon36", Z=18, N=36, a=35.97, registry=reg2)
argon_36_element.add_isotope(argon_36, 1.0)

argon_36_material = pg4.geant4.Material(
    name="LiquidArgon36",
    density=1.396,  # g/cm3
    number_of_components=1,
    state="liquid",
    temperature=88.8,  # K
    pressure=1.0 * 1e5,  # pascal
    registry=reg2,
)
argon_36_material.add_element_massfraction(argon_36_element, 1.0)

mat_s = pg4.geant4.solid.Orb("material", 1999.99, registry=reg2, lunit="m")
mat_l = pg4.geant4.LogicalVolume(mat_s, argon_36_material, "material", registry=reg2)
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], mat_l, "material", world_l, registry=reg2
)
write_pygeom(reg2, "gdml/largon_36_world.gdml")

#!/bin/env python3

from __future__ import annotations

from pyg4ometry import geant4 as g4
from pygeomtools import RemageDetectorInfo, write_pygeom

# start with the world
registry = g4.Registry()
world = g4.solid.Box("world", 2, 2, 2, registry, "m")
world_lv = g4.LogicalVolume(
    world, g4.MaterialPredefined("G4_Galactic"), "world", registry
)
registry.setWorld(world_lv)

# create a simple world with three detectors inside
basic_orb = g4.solid.Orb("basic_orb", 0.4, registry, "m")
det1 = g4.LogicalVolume(
    basic_orb, g4.MaterialPredefined("G4_Ge"), "germanium_det1", registry
)
det2 = g4.LogicalVolume(
    basic_orb, g4.MaterialPredefined("G4_Ge"), "germanium_det2", registry
)
det3 = g4.LogicalVolume(
    basic_orb, g4.MaterialPredefined("G4_Ge"), "different_det", registry
)
g4.PhysicalVolume(
    [0, 0, 0], [-500, -250, 0], det1, "germanium_det1", world_lv, registry
).set_pygeom_active_detector(RemageDetectorInfo("germanium", 1, True, "germanium_dets"))
g4.PhysicalVolume(
    [0, 0, 0], [500, -250, 0], det2, "germanium_det2", world_lv, registry
).set_pygeom_active_detector(RemageDetectorInfo("germanium", 1, True, "germanium_dets"))
g4.PhysicalVolume(
    [0, 0, 0], [0, 500, 0], det3, "different_det", world_lv, registry
).set_pygeom_active_detector(RemageDetectorInfo("scintillator", 103))

write_pygeom(registry, "geometry2.gdml")

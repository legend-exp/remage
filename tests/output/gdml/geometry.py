#!/bin/env python3

from __future__ import annotations

from pyg4ometry import geant4 as g4
from pygeomtools import RemageDetectorInfo, write_pygeom

registry = g4.Registry()
world = g4.solid.Box("world", 2, 2, 2, registry, "m")
world_lv = g4.LogicalVolume(
    world, g4.MaterialPredefined("G4_Galactic"), "world", registry
)
registry.setWorld(world_lv)

scint = g4.solid.Box("scint", 0.5, 1, 1, registry, "m")
scint1 = g4.LogicalVolume(scint, g4.MaterialPredefined("G4_lAr"), "scint1", registry)
scint2 = g4.LogicalVolume(scint, g4.MaterialPredefined("G4_lAr"), "scint2", registry)
g4.PhysicalVolume(
    [0, 0, 0], [-255, 0, 0], scint1, "scint1", world_lv, registry
).set_pygeom_active_detector(RemageDetectorInfo("scintillator", 1))
g4.PhysicalVolume(
    [0, 0, 0], [+255, 0, 0], scint2, "scint2", world_lv, registry
).set_pygeom_active_detector(RemageDetectorInfo("scintillator", 2))

det = g4.solid.Box("det", 0.1, 0.5, 0.5, registry, "m")
det = g4.LogicalVolume(det, g4.MaterialPredefined("G4_Ge"), "det", registry)
g4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], det, "det1", scint1, registry
).set_pygeom_active_detector(RemageDetectorInfo("germanium", 11))
g4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], det, "det2", scint2, registry
).set_pygeom_active_detector(RemageDetectorInfo("germanium", 12))


write_pygeom(registry, "geometry.gdml")

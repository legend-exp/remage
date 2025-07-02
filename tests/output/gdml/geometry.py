#!/bin/env python3

from __future__ import annotations

import legendoptics.lar
import pint
from pyg4ometry import geant4 as g4
from pygeomtools import RemageDetectorInfo, write_pygeom

u = pint.get_application_registry().get()

# start with the world
registry = g4.Registry()
world = g4.solid.Box("world", 2, 2, 2, registry, "m")
world_lv = g4.LogicalVolume(
    world, g4.MaterialPredefined("G4_Galactic"), "world", registry
)
registry.setWorld(world_lv)

# two outer shells: optical detectors
optdet = g4.solid.Box("optdet", 0.52, 1.02, 1.02, registry, "m")
optdet1 = g4.LogicalVolume(optdet, g4.MaterialPredefined("G4_Si"), "optdet1", registry)
optdet2 = g4.LogicalVolume(optdet, g4.MaterialPredefined("G4_Si"), "optdet2", registry)
g4.PhysicalVolume(
    [0, 0, 0], [-270, 0, 0], optdet1, "optdet1", world_lv, registry
).set_pygeom_active_detector(RemageDetectorInfo("optical", 101))
g4.PhysicalVolume(
    [0, 0, 0], [+270, 0, 0], optdet2, "optdet2", world_lv, registry
).set_pygeom_active_detector(RemageDetectorInfo("optical", 102))

# need to set some optical properties to make them sensitive to photons
surf_to_sipm = g4.solid.OpticalSurface(
    "surface_to_sipm",
    finish="ground",
    model="unified",
    surf_type="dielectric_metal",
    value=0,
    registry=registry,
)
surf_to_sipm.addVecProperty("EFFICIENCY", [1, 10], [1, 1])
surf_to_sipm.addVecProperty("REFLECTIVITY", [1, 10], [0, 0])

g4.SkinSurface("surface_sipm1", optdet1, surf_to_sipm, registry)
g4.SkinSurface("surface_sipm2", optdet2, surf_to_sipm, registry)

# these optical detectors wrap a LAr volume
# need to create a scintillating liquid argon material
lar = g4.Material(
    name="liquid_argon",
    density=1.390,  # g/cm3
    number_of_components=1,
    state="liquid",
    temperature=88.8,
    pressure=1.0 * 1e5,  # pascal
    registry=registry,
)
lar.add_element_natoms(g4.ElementSimple("argon", "Ar", 18, 39.95, registry), natoms=1)

legendoptics.lar.pyg4_lar_attach_rindex(
    lar,
    registry,
)
legendoptics.lar.pyg4_lar_attach_attenuation(
    lar,
    registry,
    88.8 * u.K,
)
legendoptics.lar.pyg4_lar_attach_scintillation(
    lar,
    registry,
    flat_top_yield=1000 / u.MeV,  # lowered scintillation yield for performance
    triplet_lifetime_method="legend200-llama",
)

# now create and place the scintillators
scint = g4.solid.Box("scint", 0.5, 1, 1, registry, "m")
scint1 = g4.LogicalVolume(scint, lar, "scint1", registry)
scint2 = g4.LogicalVolume(scint, lar, "scint2", registry)
g4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], scint1, "scint1", optdet1, registry
).set_pygeom_active_detector(RemageDetectorInfo("scintillator", 1))
g4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], scint2, "scint2", optdet2, registry
).set_pygeom_active_detector(RemageDetectorInfo("scintillator", 2))

# finally, immersed in liquid argon, germanium detectors
det = g4.solid.Box("det", 0.1, 0.5, 0.5, registry, "m")
det = g4.LogicalVolume(det, g4.MaterialPredefined("G4_Ge"), "det", registry)
g4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], det, "det1", scint1, registry
).set_pygeom_active_detector(RemageDetectorInfo("germanium", 11))
g4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], det, "det2", scint2, registry
).set_pygeom_active_detector(RemageDetectorInfo("germanium", 12))

# from pyg4ometry import visualisation as vis
# v = vis.VtkViewer()
# v.addLogicalVolume(registry.getWorldVolume())
# v.setCameraFocusPosition(focalPoint=[0, 0, 0], position=[1, 0, 0])
# v.view()

write_pygeom(registry, "geometry.gdml")

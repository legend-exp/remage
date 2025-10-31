from __future__ import annotations

import legendhpges as hpges
import pyg4ometry as pg4
from pygeomtools import RemageDetectorInfo, write_pygeom

reg = pg4.geant4.Registry()

# BEGe detector
bege_meta = {
    "name": "B00000B",
    "type": "bege",
    "production": {
        "enrichment": {"val": 0.874, "unc": 0.003},
        "mass_in_g": 697.0,
    },
    "geometry": {
        "height_in_mm": 29.46,
        "radius_in_mm": 36.98,
        "groove": {"depth_in_mm": 2.0, "radius_in_mm": {"outer": 10.5, "inner": 7.5}},
        "pp_contact": {"radius_in_mm": 7.5, "depth_in_mm": 0},
        "taper": {
            "top": {"angle_in_deg": 0.0, "height_in_mm": 0.0},
            "bottom": {"angle_in_deg": 0.0, "height_in_mm": 0.0},
        },
    },
}

# Define the world
world_radius = 6  # cm
world_s = pg4.geant4.solid.Orb("World_s", world_radius, registry=reg, lunit="cm")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "World_L", registry=reg)
reg.setWorld(world_l)

bege_l = hpges.make_hpge(bege_meta, name="BEGe_L", registry=reg)

# Place the BEGe detector at the center
bege_pv = pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], bege_l, "BEGe", world_l, registry=reg
)

# Register as sensitive detector (for Remage)
bege_pv.pygeom_active_detector = RemageDetectorInfo("germanium", 1, bege_meta)

bege_transform = [[0, 0, 0], [0, 0, -0]]
lar_radius = 6
lar_s = pg4.geant4.solid.Orb("LAr_s", lar_radius, registry=reg, lunit="cm")
lar_minus_bege_s = pg4.geant4.solid.Subtraction(
    "LAr_minus_BEGe", lar_s, bege_l.solid, bege_transform, registry=reg
)

# Create the liquid Argon balloon (world-sized for convenience)
lar_l = pg4.geant4.LogicalVolume(lar_minus_bege_s, "G4_lAr", "LAr_L", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "LAr", world_l, registry=reg)
source_l = pg4.geant4.LogicalVolume(
    lar_minus_bege_s, "G4_lAr", "Source_L", registry=reg
)

# Place it at the center of the world
pg4.geant4.PhysicalVolume(
    [0, 0, 0], [0, 0, 0], source_l, "Source", world_l, registry=reg
)

write_pygeom(reg, "gdml/geometry.gdml")

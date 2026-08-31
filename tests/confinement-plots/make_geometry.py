#!/usr/bin/env python
"""Build a simple geometry with two BEGe detectors stacked above each other."""

from __future__ import annotations

import argparse
from pathlib import Path

import pyg4ometry as pg4
import pygeomtools
from numpy import pi
from pygeomhpges import make_hpge

# BEGe metadata, as in the basic tutorial
bege_meta = {
    "name": "B00000B",
    "type": "bege",
    "production": {
        "enrichment": {"val": 0.874, "unc": 0.003},
        "mass_in_g": 480.0,
    },
    "geometry": {
        "height_in_mm": 22.0,
        "radius_in_mm": 36.98,
        "groove": {"depth_in_mm": 2.0, "radius_in_mm": {"outer": 10.5, "inner": 7.5}},
        "pp_contact": {"radius_in_mm": 7.5, "depth_in_mm": 0},
        "taper": {
            # a generous 45 deg taper on the top corner, to make the detector
            # shape a bit more interesting than a plain cylinder
            "top": {"angle_in_deg": 45.0, "height_in_mm": 6.0},
            "bottom": {"angle_in_deg": 0.0, "height_in_mm": 0.0},
        },
    },
}

# vertical distance between the centers of the two detectors
DETECTOR_PITCH_IN_MM = 32

# liquid argon cylinder
LAR_RADIUS_IN_MM = 80
LAR_HEIGHT_IN_MM = 150


def make_registry() -> pg4.geant4.Registry:
    reg = pg4.geant4.Registry()

    # world
    world_s = pg4.geant4.solid.Box("World_s", 1, 1, 1, registry=reg, lunit="m")
    world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "World_l", registry=reg)
    reg.setWorld(world_l)

    # liquid argon bath
    lar_s = pg4.geant4.solid.Tubs(
        "LAr_s",
        0,
        LAR_RADIUS_IN_MM,
        LAR_HEIGHT_IN_MM,
        0,
        2 * pi,
        registry=reg,
        lunit="mm",
    )
    lar_l = pg4.geant4.LogicalVolume(lar_s, "G4_lAr", "LAr_l", registry=reg)
    lar_l.pygeom_color_rgba = (0, 0, 1, 0.1)
    pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "LAr", world_l, registry=reg)

    # the two BEGes, stacked along z. Both share the same logical volume: the
    # detector metadata is identical, only the placement differs.
    bege_l = make_hpge(bege_meta, name="BEGe_l", registry=reg)
    bege_l.pygeom_color_rgba = (0, 1, 1, 1)

    # legend-pygeom-hpges puts the origin of the HPGe solid on its bottom
    # surface: shift the placements so the detectors are centered on +-pitch/2
    z_offset = -bege_meta["geometry"]["height_in_mm"] / 2

    for uid, (name, z) in enumerate(
        [
            ("BEGe_top", +DETECTOR_PITCH_IN_MM / 2 + z_offset),
            ("BEGe_bottom", -DETECTOR_PITCH_IN_MM / 2 + z_offset),
        ],
        start=1,
    ):
        pv = pg4.geant4.PhysicalVolume(
            [0, 0, 0], [0, 0, z], bege_l, name, lar_l, registry=reg
        )
        # register as sensitive detector in remage and store the metadata
        pv.set_pygeom_active_detector(
            pygeomtools.RemageDetectorInfo("germanium", uid, bege_meta)
        )

    return reg


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        default=str(Path(__file__).parent / "gdml" / "bege-stack.gdml"),
        help="output GDML file",
    )
    args = parser.parse_args()

    reg = make_registry()

    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    pygeomtools.write_pygeom(reg, args.output)
    print(f"wrote {args.output}")

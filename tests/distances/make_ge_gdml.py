from __future__ import annotations

import json
from pathlib import Path

import numpy as np
import pyg4ometry as pg4
import pygeomtools as pytools
from legendhpges import make_hpge

# read the configs
out_gdml = "gdml/ge-array.gdml"
det_macro = "macros/detectors-fake.mac"
config_dict = {}
for det_type in ["B", "P", "V", "C"]:
    with Path.open(Path(f"macros/{det_type}99000A.json")) as file:
        config_dict[det_type] = json.load(file)


def add_hpge(lar, reg, angle, radius, z, idx, dtype):
    x = radius * np.sin(np.deg2rad(angle))
    y = radius * np.cos(np.deg2rad(angle))
    logical_detector = make_hpge(config_dict[dtype], name=f"{dtype}{idx}", registry=reg)
    logical_detector.pygeom_color_rgba = (0, 1, 1, 0.2)
    physical_detector = pg4.geant4.PhysicalVolume(
        [0, 0, 0], [x, y, z], logical_detector, f"{dtype}{idx}", lar, reg
    )

    physical_detector.pygeom_active_dector = pytools.RemageDetectorInfo(
        "germanium",
        idx,
        config_dict[dtype],
    )
    return idx + 1


# construct geometry
reg = pg4.geant4.Registry()
ws = pg4.geant4.solid.Box("ws", 500, 500, 500, reg, lunit="mm")
wl = pg4.geant4.LogicalVolume(ws, "G4_Galactic", "wl", reg)
wl.pygeom_color_rgba = (0.1, 1, 0.1, 0.5)

reg.setWorld(wl)


# lar
lar_s = pg4.geant4.solid.Tubs(
    "LAr_s", 0, 200, 250, 0, 2 * np.pi, registry=reg, lunit="mm"
)
lar_l = pg4.geant4.LogicalVolume(lar_s, "G4_lAr", "LAr_l", registry=reg)
lar_l.pygeom_color_rgba = (1, 0.1, 0, 0.2)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "LAr", wl, registry=reg)


# hpge strings
string_radius = 85
string_angles = [0, 90, 180, 270]
detectors = ["V", "P", "B", "C"]


n = 0
lines = []
for i, det in enumerate(detectors):
    angle = string_angles[i]
    n = add_hpge(lar_l, reg, angle, string_radius, -20, n, det)

    x = string_radius * np.sin(np.deg2rad(angle))
    y = string_radius * np.cos(np.deg2rad(angle))

    lines.append("/RMG/Generator/Confinement/Geometrical/AddSolid Cylinder ")
    lines.append(f"/RMG/Generator/Confinement/Geometrical/CenterPositionX {x} mm")
    lines.append(f"/RMG/Generator/Confinement/Geometrical/CenterPositionY {y} mm ")
    lines.append("/RMG/Generator/Confinement/Geometrical/CenterPositionZ 0 mm ")
    lines.append("/RMG/Generator/Confinement/Geometrical/Cylinder/OuterRadius 44 mm ")
    lines.append("/RMG/Generator/Confinement/Geometrical/Cylinder/Height 100 mm \n")

lines_exclude = [line.replace("AddSolid", "AddExcludedSolid") for line in lines]


pytools.detectors.write_detector_auxvals(reg)
pytools.geometry.check_registry_sanity(reg, reg)


w = pg4.gdml.Writer()
w.addDetector(reg)
w.write(out_gdml)
pytools.detectors.generate_detector_macro(reg, det_macro)

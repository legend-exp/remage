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
heights = {}
for det_type in ["B", "P", "V", "C"]:
    with Path.open(Path(f"macros/{det_type}99000A.json")) as file:
        config_dict[det_type] = json.load(file)
        heights[det_type] = config_dict[det_type]["geometry"]["height_in_mm"]


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
ws = pg4.geant4.solid.Box("ws", 3000, 3000, 3000, reg, lunit="mm")
wl = pg4.geant4.LogicalVolume(ws, "G4_Galactic", "wl", reg)
wl.pygeom_color_rgba = (0.1, 1, 0.1, 0.5)

reg.setWorld(wl)


# lar
lar_s = pg4.geant4.solid.Tubs(
    "LAr_s", 0, 200, 800, 0, 2 * np.pi, registry=reg, lunit="mm"
)
lar_l = pg4.geant4.LogicalVolume(lar_s, "G4_lAr", "LAr_l", registry=reg)
lar_l.pygeom_color_rgba = (1, 0.1, 0, 0.2)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "LAr", wl, registry=reg)


# hpge strings
string_radius = 85
string_angles = [0, 80, 150, 220, 290]
offset = 200

n = 0
det_info = [
    {
        "angle": string_angles[0],
        "radius": string_radius,
        "detectors": ["V", "V", "V", "P"],
        "offsets": [5, 10, 3, 5],
    },
    {
        "angle": string_angles[1],
        "radius": string_radius,
        "detectors": ["B", "B", "B", "B", "B", "B", "P"],
        "offsets": [5, 4, 4, 3, 2, 5, 7],
    },
    {
        "angle": string_angles[2],
        "radius": string_radius,
        "detectors": ["V", "V", "B", "B"],
        "offsets": [10, 8, 7, 2],
    },
    {
        "angle": string_angles[3],
        "radius": string_radius,
        "detectors": ["V", "V", "V"],
        "offsets": [4, 3, 10],
    },
    {
        "angle": string_angles[4],
        "radius": string_radius,
        "detectors": ["B", "C", "C", "V"],
        "offsets": [3, 7, 8, 5],
    },
]


n = 0
lines = []
for string in det_info:
    angle = string["angle"]
    radius = string["radius"]
    z = offset
    for d, o in zip(string["detectors"], string["offsets"]):
        z -= heights[d]
        n = add_hpge(lar_l, reg, angle, radius, z, n, d)
        z -= o

    x = radius * np.sin(np.deg2rad(angle))
    y = radius * np.cos(np.deg2rad(angle))

    lines.append("/RMG/Generator/Confinement/Geometrical/AddSolid Cylinder ")
    lines.append(f"/RMG/Generator/Confinement/Geometrical/CenterPositionX {x} mm")
    lines.append(f"/RMG/Generator/Confinement/Geometrical/CenterPositionY {y} mm ")
    lines.append("/RMG/Generator/Confinement/Geometrical/CenterPositionZ 50 mm ")
    lines.append("/RMG/Generator/Confinement/Geometrical/Cylinder/OuterRadius 44 mm ")
    lines.append("/RMG/Generator/Confinement/Geometrical/Cylinder/Height 400 mm \n")

lines_exclude = [line.replace("AddSolid", "AddExcludedSolid") for line in lines]

# write the coordinates of the lar volumes
with Path("macros/lar-in-coordinates.mac").open("w") as f:
    for line in lines:
        f.write(line + "\n")

with Path("macros/lar-out-coordinates.mac").open("w") as f:
    for line in lines_exclude:
        f.write(line + "\n")

pytools.detectors.write_detector_auxvals(reg)
pytools.geometry.check_registry_sanity(reg, reg)


w = pg4.gdml.Writer()
w.addDetector(reg)
w.write(out_gdml)
pytools.detectors.generate_detector_macro(reg, det_macro)

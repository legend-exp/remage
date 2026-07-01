"""Geometry for the depth-profile validation test.

A single large axis-aligned cube centered at the origin. The cube half-size
(200 mm) is far larger than the sampled depths (a few mm), so edge/corner
over-counting of the depth profile is negligible and the reconstructed depth
distribution can be compared cleanly against the configured PDF.
"""

from __future__ import annotations

import pyg4ometry as pg4

out_gdml = "gdml/depth-box.gdml"

reg = pg4.geant4.Registry()

# world
ws = pg4.geant4.solid.Box("ws", 1000, 1000, 1000, reg, lunit="mm")
wl = pg4.geant4.LogicalVolume(ws, "G4_Galactic", "wl", reg)
wl.pygeom_color_rgba = (0.1, 1, 0.1, 0.2)
reg.setWorld(wl)

# a single cube (full side 400 mm -> half-size 200 mm) centered at the origin
box = pg4.geant4.solid.Box("box", pX=400, pY=400, pZ=400, registry=reg, lunit="mm")
box_l = pg4.geant4.LogicalVolume(box, "G4_lAr", "box", reg)
box_l.pygeom_color_rgba = (0.7, 1, 0.5, 1)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], box_l, "box", wl, reg)

w = pg4.gdml.Writer()
w.addDetector(reg)
w.write(out_gdml)

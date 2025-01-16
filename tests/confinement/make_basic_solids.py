from __future__ import annotations

import numpy as np
import pint
import pyg4ometry as pg4

u = pint.UnitRegistry()

# read the configs

out_gdml = "gdml/simple-solids.gdml"


reg = pg4.geant4.Registry()
ws = pg4.geant4.solid.Box("ws", 1000, 1000, 300, reg, lunit="mm")
wl = pg4.geant4.LogicalVolume(ws, "G4_lAr", "wl", reg)
wl.pygeom_color_rgba = (0.1, 1, 0.1, 0.5)
reg.setWorld(wl)


# tubby
tubby = pg4.geant4.solid.Tubs(
    "tubby",
    pRMin=0,
    pRMax=60,
    pDz=60,
    pSPhi=0,
    pDPhi=2 * np.pi,
    registry=reg,
    lunit="mm",
)
tubby_l = pg4.geant4.LogicalVolume(tubby, "G4_ADIPOSE_TISSUE_ICRP", "tubby", reg)
tubby_l.pygeom_color_rgba = (1, 0.2, 1, 1)
tubby_p = pg4.geant4.PhysicalVolume([0, 0, 0], [-200, 0, 0], tubby_l, "tubby", wl, reg)

# box
box = pg4.geant4.solid.Box("box", pX=50, pY=50, pZ=100, registry=reg, lunit="mm")
box_l = pg4.geant4.LogicalVolume(box, "G4_CHLOROFORM", "box", reg)
box_l.pygeom_color_rgba = (0.7, 1, 0.5, 1)
box_p = pg4.geant4.PhysicalVolume([0, 0, 0], [-50, 0, 0], box_l, "box", wl, reg)


# trd
trd = pg4.geant4.solid.Trd(
    "trd", pDx1=50, pDx2=10, pDy1=50, pDy2=10, pDz=100, lunit="mm", registry=reg
)
trd_l = pg4.geant4.LogicalVolume(trd, "G4_TESTIS_ICRP", "trd", reg)
trd_l.pygeom_color_rgba = (0.2, 0.2, 0.5, 1)
trd_p = pg4.geant4.PhysicalVolume([0, 0, 0], [100, 0, 0], trd_l, "trd", wl, reg)


# cone
con = pg4.geant4.solid.Cons(
    "con",
    pRmin2=0,
    pRmax2=0,
    pRmin1=0,
    pRmax1=60,
    pDz=100,
    pSPhi=0,
    pDPhi=2 * np.pi,
    registry=reg,
    lunit="mm",
)
con_l = pg4.geant4.LogicalVolume(con, "G4_BONE_CORTICAL_ICRP", "con", reg)
con_l.pygeom_color_rgba = (0.0, 0.2, 0.8, 1)
con_p = pg4.geant4.PhysicalVolume([0, 0, 0], [-200, 150, 0], con_l, "con", wl, reg)

# subtraction
small_tubby = pg4.geant4.solid.Tubs(
    "small_tubby",
    pRMin=0,
    pRMax=20,
    pDz=60,
    pSPhi=0,
    pDPhi=2 * np.pi,
    lunit="mm",
    registry=reg,
)
sub = pg4.geant4.solid.Subtraction(
    "sub", tubby, small_tubby, tra2=[[0, 0, 0], [0, 0, 0]], registry=reg
)
sub_l = pg4.geant4.LogicalVolume(sub, "G4_UREA", "sub", reg)
sub_l.pygeom_color_rgba = (1, 0.2, 0.5, 1)
sub_p = pg4.geant4.PhysicalVolume([0, 0, 0], [-50, 150, 0], sub_l, "sub", wl, reg)

small_box = pg4.geant4.solid.Box(
    "small_box", pX=20, pY=20, pZ=50, registry=reg, lunit="mm"
)
uni = pg4.geant4.solid.Union(
    "uni", box, small_box, tra2=[[0, 0, 0], [0, 0, 75]], registry=reg
)
uni_l = pg4.geant4.LogicalVolume(uni, "G4_BLOOD_ICRP", "uni", reg)
uni_l.pygeom_color_rgba = (1, 0.2, 0.1, 1)
uni_p = pg4.geant4.PhysicalVolume([0, 0, 0], [100, 150, 0], uni_l, "uni", wl, reg)


w = pg4.gdml.Writer()
w.addDetector(reg)
w.write(out_gdml)

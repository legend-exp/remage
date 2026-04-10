from __future__ import annotations

import pyg4ometry as pg4
import pygeomhpges as hpges
import pygeomtools

for mat_str in ("ge", "ar", "cu"):
    reg = pg4.geant4.Registry()

    # create a world volume
    world_s = pg4.geant4.solid.Orb("world", 20, registry=reg, lunit="cm")
    world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
    reg.setWorld(world_l)

    # let's make a germanium balloon
    if mat_str == "ge":
        mat = hpges.materials.make_natural_germanium(reg)
    elif mat_str == "ar":
        mat = pygeomtools.materials.LegendMaterialRegistry(
            reg, enable_optical=False
        ).liquidargon
    elif mat_str == "cu":
        mat = pygeomtools.materials.LegendMaterialRegistry(
            reg, enable_optical=False
        ).metal_copper

    ge_s = pg4.geant4.solid.Orb("target", 15, registry=reg, lunit="cm")
    ge_l = pg4.geant4.LogicalVolume(ge_s, mat, "target", registry=reg)
    pg4.geant4.PhysicalVolume(
        [0, 0, 0], [0, 0, 0], ge_l, "target", world_l, registry=reg
    )

    w = pg4.gdml.Writer()
    w.addDetector(reg)
    w.write(f"geometry-{mat_str}.gdml")

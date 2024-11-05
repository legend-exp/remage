# Basic Tutorial

In this tutorial we are going to demonstrate the basic functionality of
*remage* by simulating a series of particle physics processes in a simple setup.

We need to develop a geometry, and we will be using the
[pyg4ometry](https://pyg4ometry.readthedocs.io) library for this purpose. This
library is well-suited for creating geometries that are compatible with the
Geant4 framework, as it closely mirrors the Geant4 interface, making it easy
for those familiar with Geant4 to use. Importantly, pyg4ometry is independent
of Geant4 itself, meaning it doesn't require Geant4 as a dependency.
Additionally, it offers the flexibility to export the developed geometry in
GDML format, which is widely compatible for simulations and analyses in
high-energy physics applications.

The geometry consist in two high-purity germanium detectors (HPGes) immersed in
a liquid argon balloon. The
[legend-pygeom-hpges](https://legend-pygeom-hpges.readthedocs.io) package will
help us creating the HPGe volumes. Let's start by importing the Python
packages, declaring a geometry registry and specifying the dimensions and types
of the two detectors as dictionaries:

```python
import legendhpges as hpges
import pyg4ometry as pg4
from numpy import pi


reg = pg4.geant4.Registry()

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

coax_meta = {
    "name": "C000RG1",
    "type": "coax",
    "production": {
        "enrichment": {"val": 0.855, "unc": 0.015},
    },
    "geometry": {
        "height_in_mm": 84,
        "radius_in_mm": 38.25,
        "borehole": {"radius_in_mm": 6.75, "depth_in_mm": 73},
        "groove": {"depth_in_mm": 2, "radius_in_mm": {"outer": 20, "inner": 17}},
        "pp_contact": {"radius_in_mm": 17, "depth_in_mm": 0},
        "taper": {
            "top": {"angle_in_deg": 45, "height_in_mm": 5},
            "bottom": {"angle_in_deg": 45, "height_in_mm": 2},
            "borehole": {"angle_in_deg": 0, "height_in_mm": 0},
        },
    }
}
```

Now we can build all the logical volumes and place them in the world volume:

```python
# create logical volumes for the two HPGe detectors
bege_l = hpges.make_hpge(bege_meta, name="BEGe_L", registry=reg)
coax_l = hpges.make_hpge(coax_meta, name="Coax_L", registry=reg)

# create a world volume
world_s = pg4.geant4.solid.Orb("World_s", 20, registry=reg, lunit="cm")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "World", registry=reg)
reg.setWorld(world_l)

# let's make a liquid argon balloon
lar_s = pg4.geant4.solid.Orb("LAr_s", 15, registry=reg, lunit="cm")
lar_l = pg4.geant4.LogicalVolume(lar_s, "G4_lAr", "LAr_l", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "LAr", world_l, registry=reg)

# now place the two HPGe detectors in the argon
pg4.geant4.PhysicalVolume([0, 0, 0], [5, 0, -3, "cm"], bege_l, "BEGe", lar_l, registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [-5, 0, -3, "cm"], coax_l, "Coax", lar_l, registry=reg)

# finally create a small radioactive source
source_s = pg4.geant4.solid.Tubs("Source_s", 0, 1, 1, 0, 2*pi, registry=reg)
source_l = pg4.geant4.LogicalVolume(source_s, "G4_BRAIN_ICRP", "Source_L", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 5, 0, "cm"], source_l, "Source", lar_l, registry=reg)
```

Note how we also created a small cylinder to represent a radioactive source
later in the simulation.

Now we can quickly visualize the result, still with *pyg4ometry*:

```python
# start an interactive VTK viewer instance
viewer = pg4.visualisation.VtkViewerColoured(materialVisOptions={"G4_lAr": [0, 0, 1, 0.1]})
viewer.addLogicalVolume(reg.getWorldVolume())
viewer.view()
```

![Geometry visualization](img/tutorial-pyg4-view.jpg)

By following instructions in the <project:./install.rst> section, you should
have access to the `remage` executable. We are now ready to simulate some
particle physics with it.

Like any other Geant4-based application, we need to configure the simulation
with a macro file. Standard Geant4 commands as well as custom commands (see the
<project:./rmg-commands.rst>) are available.

At the beginning of the file, we have to register the "sensitive detectors" (in
our simple case, the two HPGes and the LAr). *remage* offers several types of
predefined detectors, targeting different physical quantities of the particles
that interact with them. HPGes are of type `Germanium`, while the LAr is of
type `scintillator`. Their difference in terms of simulation output will be
clear later, while inspecting it. As per specification of the
`/RMG/Geometry/RegisterDetector` command, we need to provide a unique numeric
identifier that will be used to label the detector data in the simulation
output:

```text
/RMG/Geometry/RegisterDetector Germanium BEGe 001
/RMG/Geometry/RegisterDetector Germanium Coax 002
/RMG/Geometry/RegisterDetector Scintillator LAr 003
```

Now we can initialize the simulation. Additionally, let's setup some Geant4
visualization to look at the tracks:

```text
/run/initialize

# create a scene
/vis/open OI
/vis/scene/create
/vis/sceneHandler/attach

# draw the geometry
/vis/drawVolume

# setup better colors
/vis/viewer/set/defaultColour black
/vis/viewer/set/background white

# and also show trajectories and particle hits
/vis/scene/add/trajectories smooth
/vis/scene/add/hits
/vis/scene/endOfEventAction accumulate
```

Now with the actual physics. We want to start ten 1 MeV gammas from the
radioactive source:

```text
/RMG/Generator/Confine Volume
/RMG/Generator/Confinement/Physical/AddVolume Source

/RMG/Generator/Select GPS
/gps/particle gamma
/gps/ang/type iso
/gps/energy 1000 keV

/run/beamOn 50
```

We can finally pass the GDML geometry and the macro to the `remage` executable
and look at the result!

```console
$ remage --interactive --gdml-files geometry.gdml vis-gammas.mac
```

Interactions in HPGes and in LAr are marked in red and blue, respectively.

![Simulation visualization](img/tutorial-g4-view.jpg)

:::{tip}
With Apptainer, additional tweaks are required in order to allow for graphics to be displayed, e.g.
```console
$ apptainer run \
    --env DISPLAY=$DISPLAY \
    --env XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
    -B $XDG_RUNTIME_DIR \
    path/to/remage_latest.sif --interactive --gdml-file geometry.gdml
```
and similarly with Docker.
:::

:::{tip}
If `remage` from the Apptainer image refuses to run simulations, this might be
due to some of your environment variables from outside the container. Give
`--cleanenv` a try.
:::

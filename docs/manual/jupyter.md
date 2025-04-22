# Jupyter Example

To test if it works, open up a new notebook and select the remage kernel! You might have to search for it under "Select another Kernel" "Jupyter kernel..." depending on your installation, the name should be equal to the name under `display_name` in the `kernel.json`. Now first start with a simple cell to check if the kernel starts successfully:

```python
# Change this to the path where you want to save simulation geometry and data
path = "."
```

Make this a new code cell and run it. If everything worked the kernel should load up fine (might take some more time the first time.). Now lets see if the pygeometry dependencies will be found and we can build a geometry:

```python
from __future__ import annotations

import legendhpges as hpges
import pyg4ometry as pg4
import numpy as np

reg = pg4.geant4.Registry()

# create a world volume
world_s = pg4.geant4.solid.Orb("world", 1500, registry=reg, lunit="mm")
world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
reg.setWorld(world_l)

# lar balloon
lar_s = pg4.geant4.solid.Orb("LAr_s", 1000, registry=reg, lunit="mm")
lar_l = pg4.geant4.LogicalVolume(lar_s, "G4_lAr", "LAr_l", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], lar_l, "LAr", world_l, registry=reg)

# let's make a germanium tube
natge = hpges.materials.make_natural_germanium(reg)
ge_s = pg4.geant4.solid.Tubs(
    "germanium",
    pRMin=0,
    pRMax=100,
    pDz=50,
    pSPhi=0,
    pDPhi=2 * np.pi,
    registry=reg,
    lunit="mm",
)
ge_l = pg4.geant4.LogicalVolume(ge_s, natge, "germanium", registry=reg)
pg4.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], ge_l, "germanium", lar_l, registry=reg)

w = pg4.gdml.Writer()
w.addDetector(reg)

w.write(path + "/geometry.gdml")
```

Run this as a new code cell!

:::{note} Running the imports after a kernel is restarted can take longer than usually expected.
:::

This should create a `geometry.gdml` file in your specified directory. Let's see if we can run remage now!

```python
import subprocess
import os
import time
from pathlib import Path

macro_path = path + "/macro.mac"
macro_file = Path(macro_path)
gdml_path = path + "/geometry.gdml"

output_directory = path + "/out"
# make the out directory if it does not exist
os.makedirs(output_directory, exist_ok=True)

# Define macro content with generator placeholder
macro_content = """\
/RMG/Output/NtuplePerDetector false
/RMG/Geometry/RegisterDetector Germanium germanium 001

/run/initialize
/RMG/Processes/Stepping/ResetInitialDecayTime true

$GENERATOR

/run/beamOn 100000
"""

# Simulate normal double beta decay using the BxDecay0 module
generator = """
/RMG/Generator/Confine Volume
/RMG/Generator/Confinement/Physical/AddVolume germanium
/RMG/Generator/Select BxDecay0
/bxdecay0/generator/dbd Ge76 2345 4 0
"""

macro_content = macro_content.replace("$GENERATOR", generator)

# Write the macro file
macro_file.write_text(macro_content)

start = time.time()

result = subprocess.run(
    f"remage {macro_path} -g {gdml_path} -o {output_directory}/out.lh5 -w -t 1",
    shell=True,
)
# Check if remage encountered errors. Exit code 0 means everything worked fine. Exit code 2 are only Warnings.
if result.returncode not in {0, 2}:
    raise ValueError("remage encountered an error.")

end = time.time()
print(end - start)
```

Run this as a new code cell! You should see the usual remage print-out. Lets finish this all up with some quick analysis:

```python
from lgdo import lh5
import awkward as ak
import hist
import matplotlib.pyplot as plt

data = lh5.read_as(f"stp/germanium", f"{output_directory}/out.lh5", "ak")
# with awkward arrays, we can easily "build events" by grouping by the event identifier
evt = ak.unflatten(data, ak.run_lengths(data.evtid))
# make and plot an histogram
plt.rcParams["figure.figsize"] = (10, 3)
hist.new.Reg(440, 0, 2200, name="energy [keV]").Double().fill(
    ak.sum(evt.edep, axis=-1)
).plot(yerr=False)
plt.ylabel("counts / 5 keV")
plt.xlim(500, 2200)
plt.yscale("log")
```

If everything worked you should now be able to run the full simulation chain in a notebook, like you did just now!
![Happy visualization](https://media.tenor.com/lCKwsD2OW1kAAAAj/happy-cat-happy-happy-cat.gif)

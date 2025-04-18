(install)=

# Installation

## Pre-built binaries

The recommended and fastest way of running _remage_ is through pre-built
software containers. Stable releases are regularly made available
[on Docker Hub](https://hub.docker.com/r/legendexp/remage). To obtain and run
the latest just do:

```console
$ docker run legendexp/remage:latest --help # just prints a help message
```

If you prefer [Apptainer](https://apptainer.org/), you can easily generate an
image locally:

```console
$ apptainer build remage_latest.sif docker://legendexp/remage:latest
$ apptainer run remage_latest.sif --help
```

If containers do not work for you, see the next section to learn how to build
and install from source.

## Building from source

In preparation for the actual build, users are required to obtain some
dependencies.

```{include} ../_dependencies.md

```

### Building

The build system is based on CMake.

```{important}
It is important that you **use a unique install prefix** for remage. **Do not
use a location that is controlled by your linux package manager** (i.e.
`/usr`). The default value of `/usr/local` is also dangerous if you install
other software into that prefix. Failing to do so might create conflicts with
other python packages or might even render your system unusable.
```

```console
$ git clone https://github.com/legend-exp/remage
$ cd remage
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=<unique prefix> ..
$ make install
```

## Setting up Jupyter

Since the latest remage versions, it is possible to include remage into your jupyter python kernel. This allows you to run everything within a single jupyter notebook. Because the setup is very easy, this might be more comfortable and easier than using the CLI (depending on your experience and use-case). Keep in mind that interactive visualization, like the Pygeometry or Geant4 visualizer, might be very buggy in a notebook. You can still try it, but you can not say that we did not warn you!

To set up your jupyter kernel, you only have to provide the correct `kernel.json` file to your jupyter installation. This means you first have to find your jupyters share folder and create the `kernel.json` file there.

```console
cd ~/.local/share/jupyter
mkdir kernels
cd kernels
mkdir remage
cd remage
touch kernel.json
```

The contents of this kernel file now depends on how you installed remage.

:::{note} If you are using a jupyter that is installed inside of a virtual environment (or in vscode your default python interpreter is inside a venv) then you probably need to add the `kernel.json` to the share folder of that jupyter installation instead.
:::

### Jupyter from pre-built binaries

If you have installed remage from a pre-built binary using apptainer, your kernel file should look something like this (remember to replace `/path/to/remage_latest.sif` with your actual path):

```json
{
  "argv": [
    "apptainer",
    "exec",
    "-B",
    "/run/user/1000:/run/user/1000",
    "/path/to/remage_latest.sif",
    "python",
    "-m",
    "ipykernel_launcher",
    "-f",
    "{connection_file}"
  ],
  "display_name": "remage-latest (apptainer)",
  "language": "python",
  "metadata": {
    "debugger": true
  }
}
```

If you instead installed using docker it should look something like this: I did not get this working yet :cry:

### Jupyter from source

We are assuming that you have built remage using the `remage-base_latest` container. In this case, after `make install`-ing remage, the kernel file is almost the same as if you used the pre-built library. You only have to additionally tell the kernel where you installed remage with the export paths. The main issue here is, that `env` does not expand shell variables like `$PATH`, it only overwrites them. So we have to set them up ourself. The first path for "PATH=..." and "LD_LIBRARY_PATH..." should be the path where you installed remage. (Replace `remage/install/path/...` with the path the terminal told you to export when you did `cmake ..`). Also remember to replace `/path/to/remage_base_latest.sif` with the actual path of your image.

```json
{
  "argv": [
    "apptainer",
    "exec",
    "-B",
    "/run/user/1000:/run/user/1000",
    "/path/to/remage_base_latest.sif",
    "env",
    "PATH=/remage/install/path/bin:/opt/python-extra/bin:/opt/bxdecay0/bin:/opt/geant4/bin:/opt/root/bin:/opt/vecgeom/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
    "LD_LIBRARY_PATH=/remage/install/path/lib:/opt/bxdecay0/lib:/opt/geant4/lib:/opt/root/lib:/opt/vecgeom/lib::/.singularity.d/libs",
    "/opt/python-extra/bin/python",
    "-m",
    "ipykernel_launcher",
    "-f",
    "{connection_file}"
  ],
  "display_name": "remage-base (apptainer)",
  "language": "python",
  "metadata": {
    "debugger": true
  }
}
```

Note that this will overwrite the shell environment variables `$PATH` and `$LD_LIBRARY_PATH`.

Because that is not the prettiest solution, you could also write a small setup script instead.

:::{note} It already works with the first solution. This is an alternative solution now.
:::

For this you should write a small bash script in a folder of your liking.

```console
cd # To go back to your home directory
mkdir scripts
cd scripts
touch remage_kernel_setup.sh
chmod +x remage_kernel_setup.sh
```

In the setup script `remage_kernel_setup.sh` you should now write something like this:

```bash
#!/bin/bash
export PATH=/remage/install/path/bin:$PATH
export LD_LIBRARY_PATH=/remage/install/path/lib:$LD_LIBRARY_PATH

exec /opt/python-extra/bin/python -m ipykernel_launcher -f "$1"
```

Where again `/remage/install/path/` should be replaced with the path where you installed remage.
Your `kernel.json` should then be adjusted to look like this:

```json
{
  "argv": [
    "apptainer",
    "exec",
    "-B",
    "/run/user/1000:/run/user/1000",
    "-B",
    "/home/user/scripts/remage_kernel_setup.sh:/opt/remage/remage_kernel_setup.sh",
    "/path/to/remage_base_latest.sif",
    "/opt/remage/remage_kernel_wrapper.sh",
    "{connection_file}"
  ],
  "display_name": "remage-base (apptainer)",
  "language": "python",
  "metadata": {
    "debugger": true
  }
}
```

Remember to replace `/path/to/remage_base_latest.sif` with the actual path of your image and `/home/user/scripts/remage_kernel_setup.sh` with the actual path of the script!

### Jupyter on NERSC

Setting up the remage jupyter kernel on NERSC is exactly the same as setting up the legend-sw kernel on NERSC like explained [on Confluence](https://legend-exp.atlassian.net/wiki/spaces/LEGEND/pages/261750878/Computing+at+NERSC#Python-notebooks-in-Shifter-containers). In order to make sure the kernel does not have to first pull the remage version from Docker, maybe lets first pull it ourselves:

```console
shifterimg -v pull legendexp/remage:latest
```

The rest is literaly the same as in the Confluence explanation:

```console
$ cd ~/.local/share/jupyter
$ mkdir kernels
$ cd kernels
$ mkdir legend-base
$ cd legend-base
$ touch kernel.json
```

The only difference to the Confluence version is that in the `kernel.json` instead of "--image=legendexp/legend-software:latest" we write "--image=legendexp/remage:latest" and instead of "/opt/conda/bin/python3" we write "python". Now maybe also change the name from ""display_name": "legend-sw"," to ""display_name": "remage",". In the end your `kernel.json` should look like this:

```json
{
  "argv": [
    "shifter",
    "--module=none",
    "--image=legendexp/remage:latest",
    "python",
    "-m",
    "ipykernel_launcher",
    "-f",
    "{connection_file}"
  ],
  "display_name": "remage",
  "language": "python",
  "env": {
    "LEGENDDATADIR": "/global/cfs/projectdirs/m2676/data"
  }
}
```

Now after refreshing JupyterLab you should see the "remage" kernel in your notebooks!

### Example Jupyter file for testing

To test if it works, open up a new notebook and select the remage kernel! You might have to search for it under "Select another Kernel" "Jupyter kernel..." depending on your installation, the name should be equal to the name under "display_name" in the `kernel.json`. Now first start with a simple cell to check if the kernel starts succesfully:

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
# Check if remage encountered errors. Exit code 0 means everything worked fine. Exit code 2 are only Warnings.
if subprocess.run(
    f"remage {macro_path} -g {gdml_path} -o {output_directory}/out.lh5 -w -t 1  ",
    shell=True,
) not in {0, 2}:
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

# macro_path, gdml_path, output_directory = setup_macros_and_geometry()
# if subprocess.run(f"remage {macro_path} -g {gdml_path} -o {output_directory}/out.lh5 -w -t 1  ", shell=True,).returncode not in {0,2}:
#    raise ValueError("remage encountered an error")
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
![Geometry visualization](https://media.tenor.com/lCKwsD2OW1kAAAAj/happy-cat-happy-happy-cat.gif)

(manual-install)=

# Installation

The recommended and fastest way of running _remage_ is through pre-built
software containers. Alternatively, the project can be built from source.

## Pre-built binaries

Stable releases are regularly made available
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

More information is available in {ref}`manual-containers`. If containers do not
work for you, see the next section to learn how to build and install from
source.

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

:::{tip}

When encountering any unexpected issues the first step should always be to
remove the build folder and compile from scratch. The second step should be to
additionally also remove the install folder and any traces of _remage_ to try
the installation on a fresh system.

:::

## Setting up Jupyter

To set up a _remage_-aware Jupyter kernel, you only have to provide the correct
`kernel.json` file to your Jupyter installation.

```console
$ mkdir -p ~/.local/share/jupyter/kernels/remage
$ touch ~/.local/share/jupyter/kernels/remage/kernel.json
```

:::{note}

If Jupyter is installed inside a virtual environment (or in VSCode your default
Python interpreter is inside a virtual environment) then you probably need to
add the `kernel.json` to the `share` folder of that Jupyter installation
instead.

:::

If you have installed _remage_ from a pre-built binary using Apptainer, your
kernel file should look something like this (remember to replace
`/path/to/remage_latest.sif` with your actual path):

```json
{
  "argv": [
    "sh",
    "-c",
    "apptainer exec -B $XDG_RUNTIME_DIR:$XDG_RUNTIME_DIR /path/to/remage_latest.sif python -m ipykernel_launcher -f {connection_file}"
  ],
  "display_name": "remage container",
  "language": "python",
  "metadata": {
    "debugger": true
  }
}
```

Now after refreshing JupyterLab/VSCode you should be able to find the "remage
container" kernel in your notebooks!

:::{warning}

Interactive visualization, like the pyg4ometry or Geant4 visualizer, might not
work as expected in a notebook.

:::

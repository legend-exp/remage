---
myst:
  substitutions:
    bxdecay0: "[BxDecay0](https://github.com/BxCppDev/bxdecay0)"
    geant4: "[Geant4](https://geant4.web.cern.ch)"
    hdf5: "[HDF5](https://www.hdfgroup.org/solutions/hdf5)"
    root: "[ROOT](https://root.cern.ch)"
---

# Required dependencies

- [CMake](https://cmake.org) 3.12 or higher
- [geant4] 11.0.3 or higher

# Optional dependencies

- [geant4] support for:

  - HDF5 object persistency
  - Multithreading
  - GDML geometry description

- [root] 6.06 or higher

- [bxdecay0] 1.0.10 or higher

- [hdf5] C++ support for LH5 object persistency

:::{note}
Pre-built Docker container images with all necessary dependencies are available [on
Docker Hub](https://hub.docker.com/repository/docker/gipert/remage-base).
:::

:::{note}
Apptainer images can be easily generated with, e.g.:

```console
$ apptainer build remage-base_latest.sif docker://gipert/remage-base:latest
```

For more details, have a look at [the
documentation](https://apptainer.org/docs/user/main/build_a_container.html).
:::

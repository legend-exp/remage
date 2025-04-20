### Required dependencies

- [CMake] 3.12 or higher
- [Geant4] 11.0.3 or higher
- Python3 interpreter

### Optional dependencies

- [Geant4] support for:
  - [HDF5] object persistency
  - Multithreading
  - GDML geometry description
- [ROOT] 6.06 or higher
- [BxDecay0] 1.0.10 or higher
- [HDF5] C++ support for LH5 object persistency

:::{note} Pre-built Docker container images with all necessary dependencies are
available [on Docker Hub](https://hub.docker.com/r/gipert/remage-base).
:::

:::{note} Apptainer images can be easily generated with, e.g.:

```console
 $ apptainer build remage-base_latest.sif docker://gipert/remage-base:latest
```

For more details, have a look at
[the documentation](https://apptainer.org/docs/user/main/build_a_container.html).
:::

[CMake]: https://cmake.org
[BxDecay0]: https://github.com/BxCppDev/bxdecay0
[Geant4]: https://geant4.web.cern.ch
[HDF5]: https://www.hdfgroup.org/solutions/hdf5
[ROOT]: https://root.cern.ch

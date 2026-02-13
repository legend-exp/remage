### Required dependencies

- [CMake] 3.28 or higher
- [Geant4] 11.2.2 or higher, recommended 11.3
- Python3 interpreter

### Optional dependencies

- [Geant4] support for:
  - [HDF5] object persistency (also needed for LH5)
  - Multithreading
  - GDML geometry description
- [ROOT] 6.06 or higher
- [BxDecay0] 1.0.10 or higher
- [HDF5] C++ support for LH5 object persistency

:::{note}

Pre-built Docker container images with all necessary dependencies are available
[on Docker Hub](https://hub.docker.com/r/legendexp/remage-base) (see
{ref}`manual-containers`).

:::

[CMake]: https://cmake.org
[BxDecay0]: https://github.com/BxCppDev/bxdecay0
[Geant4]: https://geant4.web.cern.ch
[HDF5]: https://www.hdfgroup.org/solutions/hdf5
[ROOT]: https://root.cern.ch

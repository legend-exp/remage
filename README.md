<img src=".github/logo/remage-logo.png" alt="remage logo" align="left" height="200">

# remage

Simulation framework for germanium detector experiments

![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/gipert/remage?logo=git)
![GitHub Workflow Status (main)](https://img.shields.io/github/workflow/status/gipert/remage/CI/main?label=main%20branch&logo=github)
![GitHub issues](https://img.shields.io/github/issues/gipert/remage?logo=github)
![GitHub pull requests](https://img.shields.io/github/issues-pr/gipert/remage?logo=github)
![License](https://img.shields.io/github/license/gipert/remage)

🚧 Work in progress...

<p></p>

The *remage* project aims to provide a modern GEANT4-based C++ library to to
efficiently simulate particle physics processes in typical germanium detector
experiments. The library is setup-agnostic, and therefore the only mandatory
user action is to provide a geometrical implementation of the experimental
setup (supported specification languages are C++, GDML and more). The user can
then benefit from a predefined set of tools to perform common actions (physics
generators, standard output classes, etc).

### Main features

* Modern CMake-based build system
* Support for modern [Geant4](https://geant4.web.cern.ch), including

    * Multithreading
    * [VecGeom](https://gitlab.cern.ch/VecGeom/VecGeom) support for vectorized
      solids
    * [GDML](https://gdml.web.cern.ch/GDML) support
    * Multiple output file formats ([ROOT](https://root.cern.ch),
      [HDF5](https://www.hdfgroup.org/solutions/hdf5)...)

* Fast third-party cosmic muon generator (through
  [EcoMug](https://doi.org/10.1016/j.nima.2021.165732))
* Third-party double-beta decay generator (through
  [bxdecay0](https://github.com/BxCppDev/bxdecay0))
* Common output schemes for HPGe and optical detectors
* GPU-offloading of optical photon tracking (through
  [Opticks](https://doi.org/10.1051/epjconf/201921402027))

<img src=".github/logo/remage-logo.png" alt="remage logo" align="left" height="200">

# remage

Simulation framework for germanium detector experiments

![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/legend-exp/remage?logo=git)
[![GitHub Workflow Status](https://img.shields.io/github/checks-status/legend-exp/remage/main?label=main%20branch&logo=github)](https://github.com/legend-exp/remage/actions)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
![GitHub issues](https://img.shields.io/github/issues/legend-exp/remage?logo=github)
![GitHub pull requests](https://img.shields.io/github/issues-pr/legend-exp/remage?logo=github)
![License](https://img.shields.io/github/license/legend-exp/remage)
[![Read the Docs](https://img.shields.io/readthedocs/remage?logo=readthedocs)](https://remage.readthedocs.io)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.11115662.svg)](https://doi.org/10.5281/zenodo.11115662)

<br/>

The *remage* project aims to provide a modern Geant4-based C++ library to
efficiently simulate particle physics processes in typical germanium detector
experiments. The library is setup-agnostic, and therefore the only mandatory
user action is to provide a geometrical implementation of the experimental
setup (supported specification languages are C++, GDML and more). The user can
then benefit from a predefined set of tools to perform common actions (physics
generators, standard output classes, etc).

### Main features

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
* Advanced vertex confinement on physical volumes, geometrical solids, surfaces and intersections
* Common output schemes for HPGe and optical detectors

<img src=".github/logo/remage-logo.png" alt="remage logo" align="left" height="200">

# remage

Simulation framework for germanium detector experiments

![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/legend-exp/remage?logo=git)
[![GitHub Workflow Status](https://img.shields.io/github/checks-status/legend-exp/remage/main?label=main%20branch&logo=github)](https://github.com/legend-exp/remage/actions)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
![GitHub issues](https://img.shields.io/github/issues/legend-exp/remage?logo=github)
![GitHub pull requests](https://img.shields.io/github/issues-pr/legend-exp/remage?logo=github)
[![Read the Docs](https://img.shields.io/readthedocs/remage?logo=readthedocs)](https://remage.readthedocs.io)
[![Docker Hub](https://img.shields.io/badge/Docker-Hub-blue?logo=docker)](https://hub.docker.com/r/legendexp/remage)
![License](https://img.shields.io/github/license/legend-exp/remage)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.11115662.svg)](https://doi.org/10.5281/zenodo.11115662)

<br/>

The _remage_ project aims to deliver a modern Geant4-based C++ library designed
for efficient simulation of particle physics processes in typical germanium
detector experiments. The library is setup-agnostic, requiring users only to
define the experimental setup's geometry. Supported specification languages
include C++, GDML, and others. Once the geometry is provided, users can access a
comprehensive suite of tools for common tasks, such as physics generation and
standard output handling.

Get started with our [documentation pages](https://remage.readthedocs.io)!

### Main features

- Low entry barrier: Most simulations can be executed directly using the
  `remage` executable and a macro file, eliminating the need to write or compile
  C++ code.
- Various pre-compiled _remage_ versions available on
  [Docker Hub](https://hub.docker.com/repository/docker/legendexp/remage)
- Support for modern [Geant4](https://geant4.web.cern.ch), including:
  - Multithreading
  - [GDML](https://gdml.web.cern.ch/GDML) support
  - Multiple output file formats ([ROOT](https://root.cern.ch),
    [HDF5](https://www.hdfgroup.org/solutions/hdf5)...)
- [LEGEND HDF5 (LH5)](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/)
  output format
- Fast third-party cosmic muon generator (through
  [EcoMug](https://doi.org/10.1016/j.nima.2021.165732))
- Support for external generators:
  - [MUSUN](https://doi.org/10.1016/j.cpc.2008.10.013)
  - [MAURINA](https://doi.org/10.1140/epja/s10050-024-01336-0)
- Third-party double-beta decay generator (through
  [bxdecay0](https://github.com/BxCppDev/bxdecay0))
- Advanced vertex confinement on physical volumes, geometrical solids, surfaces
  and intersections
- Sensible output schemes for HPGe and optical detectors

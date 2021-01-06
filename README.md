<img src=".github/remage-logo.png" alt="remage logo" align="left" height="200">

# remage

Simulation framework for germanium detector experiments

![CI](https://github.com/gipert/remage/workflows/CI/badge.svg)

🚧 Work in progress...

<p></p>

The remage project provides a modern Geant4-based C++ library that can be used
to efficiently simulate physics processes in typical germanium detector
experiments. The library is setup-agnostic, therefore the user is mainly asked
to provide a geometrical implementation of his/her experimental setup (C++
code and GDML among the supported formats). The user can then benefit from a
set of tools to perform common actions (frequently used materials, geometry
helpers, standard output classes, LAr scintillation mechanism, and much more).

### Prerequisites
* [CMake]() (≥ v3.8) — Build-only dependency
* [Geant4](https://geant4.web.cern.ch) (≥ v10.5) — with:
    * `GEANT4_INSTALL_DATA=ON`
    * `GEANT4_USE_HDF5=ON` [optional]
    * `GEANT4_USE_USOLIDS=ON` [optional], requires VecGeom library
    * `GEANT4_USE_GDML=ON` [optional]
    * `GEANT4_BUILD_MULTITHREADED=ON` [optional]

### Optional dependencies
* [bxdecay0](https://github.com/BxCppDev/bxdecay0) (≥ v1.0) with
  `BXDECAY0_WITH_GEANT4_EXTENSION=ON`, to simulate double-beta decay (and
  more...)
* [ROOT](https://root.cern.ch), only for output formatting
* [VecGeom](https://gitlab.cern.ch/VecGeom/VecGeom), for vectorized geometry
  (Geant4 must be compiled with `GEANT4_USE_USOLIDS=ON` too)

### Installing the project
To build and install the project, first clone the repository:
```console
$ git clone git@github.com:legend-exp/remage.git
```
Create a build directory, and run CMake from there:
```console
$ mkdir -p build && cd build
$ cmake <path-to-remage-source> -DCMAKE_INSTALL_PREFIX=<your-custom-install-location>
```
The default build type is set to `RelWithDebInfo`, if you need something
different you can customize it by setting the
[`CMAKE_BUILD_TYPE`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)
variable. Finally build and install the project (in `/usr/local` by default):
```console
$ cmake --build . --target install
```
and you're done!

### A simple application

Under construction... Browse the `examples` directory in the meanwhile.

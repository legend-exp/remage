# remage - simulation framework for HPGe-based experiments

![CI](https://github.com/gipert/remage/workflows/CI/badge.svg)

Wrok in progress...

### Prerequisites
* [CMake]() (≥ v3.0) — Build time dependency
* [ROOT](https://root.cern.ch) (≥ v6.06) — Built with CMake and `xml=ON`
* [Geant4](https://geant4.web.cern.ch) (≥ v10.4.3) — with `GEANT4_INSTALL_DATA=ON`, `GEANT4_USE_GDML=ON` and `GEANT4_BUILD_MULTITHREADED=OFF`

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
If you also want to build the test suite just set the `BUILD_TESTS` variable to
`ON` (see [`test/README.md`](test/README.md) for details). The default build
type is set to `RelWithDebInfo`, if you need something different you can
customize it by setting the
[`CMAKE_BUILD_TYPE`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)
variable. Finally build and install the project (in `/usr/local` by default):
```console
$ make install
```
and you're done!

### Known issues
CMake could have troubles in finding ROOT on some environments because of two
possible reasons:
* ROOT is compiled with the deprecated `./configure` method. Solution: just
  recompile it with CMake.
* the ROOT CMake files in your installation are somewhat misplaced. Solution:
  just set the `CMAKE_PREFIX_PATH` variable to point to the `cmake` folder in
  your `ROOTSYS` directory:
  ```console
  $ cmake <path-to-MaGe-source> -DCMAKE_INSTALL_PREFIX=<your-custom-install-location> -DCMAKE_PREFIX_PATH=$ROOTSYS/[etc]/cmake
  ```
* The documents build step could fail if the LaTeX executables are found but
  the distribution does not actually contain the needed packages. Turn off
  documents building by setting `BUILD_DOCS` to `OFF` in the CMake invocation.

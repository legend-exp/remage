# remage

_remage_ is a modern C++ simulation framework for germanium experiments.

## Quick start

The installation process is documented in {doc}`install`.

:::{warning}
A proper user guide is not available yet. In the meanwhile, users can have a
look at the {doc}`tutorial` or the provided
[examples](https://github.com/legend-exp/remage/tree/main/examples).
:::

In the simplest application, the user can simulate in an existing GDML geometry
through the `remage` executable:

```console
$ remage --help
remage: simulation framework for germanium experiments
Usage: ./src/remage [OPTIONS] [macros...]

Positionals:
  macros FILE ...             Macro files

Options:
  -h,--help                   Print this help message and exit
  -q                          Print only warnings and errors
  -v [0]                      Increase verbosity
  -l,--log-level LEVEL [summary]
                              Logging level debug|detail|summary|warning|error|fatal|nothing
  -i,--interactive            Run in interactive mode
  -t,--threads INT            Number of threads
  -g,--gdml-files FILE ...    GDML files
  -o,--output-file FILE       Output file for detector hits
```

Macro files can use all available upstream Geant4 macro commands, as well as the
[remage macro interface](./rmg-commands)

Advanced applications can extend _remage_ and link against `libremage` with the
usual CMake syntax:

```cmake
project(myapp)
find_package(remage)
# add_library(myapp ...)
# add_executable(myapp ...)
target_link_libraries(myapp PRIVATE RMG::remage)
```

## Next steps

```{toctree}
:maxdepth: 2

install
tutorial
rmg-commands
Output structure & LH5 output <output>
```

```{toctree}
:caption: Development
:maxdepth: 1

Developer's guide <developer>
api/index
Good ol' Doxygen <https://remage.readthedocs.io/en/latest/doxygen/annotated.html>
Source Code <https://github.com/legend-exp/remage>
License <https://github.com/legend-exp/remage/blob/main/LICENSE>
Citation <https://doi.org/10.5281/zenodo.11115662>
```

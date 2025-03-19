# remage

_remage_ is a modern C++ simulation framework for germanium experiments.

## Quick start

The installation process is documented in {doc}`manual/install`.

```{warning}
 A proper user guide is not available yet. In the meanwhile, users can have a
look at the {doc}`tutorial` or the provided
[examples](https://github.com/legend-exp/remage/tree/main/examples).
```

In the simplest application, the user can simulate in an existing GDML geometry
through the `remage` executable:

```console
$ remage --help
remage: simulation framework for germanium experiments 

remage [OPTIONS] [macros...]

POSITIONALS:
  macros FILE ...             One or more remage/Geant4 macro command listings to execute 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -q,     --quiet             Print only warnings and errors (same as --log-level=warning) 
  -v,     --verbose [0]       Increase program verbosity to maximum (same as --log-level=debug) 
          --version           Print remage's version and exit 
          --version-rich      Print versions of remage and its dependencies and exit 
  -l,     --log-level LEVEL [summary]  
                              Logging level debug|detail|summary|warning|error|fatal|nothing 
  -s,     --macro-substitutions TEXT ... 
                              key=value pairs of variables to substitute in macros (syntax as 
                              for Geant4 aliases) 
  -i,     --interactive       Open an interactive macro command prompt 
  -t,     --threads INT       Set the number of threads used by remage 
  -g,     --gdml-files FILE ... 
                              Supply one or more GDML files describing the experimental 
                              geometry 
  -o,     --output-file FILE  Output file for detector hits 
  -w,     --overwrite         Overwrite existing output files 

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

tutorial
manual/index
rmg-commands
```

```{toctree}
:caption: Links
:maxdepth: 1

remage validation report <https://legend-exp.github.io/remage/validation/{REMAGE_VERSION}>
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

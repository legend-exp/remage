# remage

_remage_ is a modern simulation framework for low-background physics
experiments.

```{warning}
We are working on a proper user guide ({doc}`manual/index`). In the meanwhile,
users can have a look at the {doc}`tutorial` or the provided
[examples](https://github.com/legend-exp/remage/tree/main/examples).
```

## Quick start

The installation process is documented in {doc}`manual/install`. In the simplest
application, the user can simulate particle interactions in an existing
{doc}`manual/geometry` through the `remage` executable by providing a "macro"
configuration file. Macro files can use all available upstream Geant4 macro
commands, as well as the [remage macro interface](./rmg-commands).

(related-projects)=

## Related projects

- [legend-pygeom-hpges](https://github.com/legend-exp/legend-pygeom-hpges):
  high-purity germanium detector geometries for radiation transport simulations.
- [legend-pygeom-optics](https://github.com/legend-exp/legend-pygeom-optics):
  optical properties of low-background experiments for Geant4 optical
  simulations.
- [legend-pygeom-tools](https://github.com/legend-exp/legend-pygeom-tools):
  general-purpose tools for implementing and visualizing geometries.
- [legend-pygeom-l200](https://github.com/legend-exp/legend-pygeom-l200) and
  [legend-pygeom-l1000](https://github.com/legend-exp/legend-pygeom-l1000): the
  LEGEND-200 and LEGEND-1000 geometries for radiation transport simulations,
  useful examples of complex experimental setup implementations.
- [reboost](https://github.com/legend-exp/reboost): post-processing and analysis
  of remage output.
- [revertex](https://github.com/legend-exp/revertex): generator for input vertex
  files for remage.

## Next steps

```{toctree}
:maxdepth: 1

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
Python API reference <api/modules>
C++ API reference <api/index>
Good ol' Doxygen <https://remage.readthedocs.io/en/latest/doxygen/annotated.html>
Source Code <https://github.com/legend-exp/remage>
License <https://github.com/legend-exp/remage/blob/main/LICENSE>
Citation <https://doi.org/10.5281/zenodo.11115662>
```

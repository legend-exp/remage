# Experimental geometry

:::{todo}

- remage design principle and pointers to pyg4ometry and friends
- how to input a GDML geometry
- navigating the geometry tree within remage
  :::

## Registering sensitive detectors

It is important to register all sensitive detector volumes that should store
particle interactions into output files. In remage, there are multiple ways
to register such a sensitive volume. Internally, each sensitive detector has
an id number (`uid`, whereas the `u` stands for unique. However, an id can
also be explicietely reused). Only one detector can be attached to each
physical volume.

The detector has a _type_, corresponding to the output scheme that will be
used for actually creating the output.

:::{note}
At present, it is not possible to register custom detector types at runtime.
:::

The most simple ways is to add
<project:../rmg-commands.md#rmggeometryregisterdetector> to the macro file for
each detector physvol to register.

Alternatively, all detectors from a type (or even all types at once) can be
imported from the
[detector mapping inside the GDML](https://legend-pygeom-tools.readthedocs.io/en/stable/metadata.html)
file with <project:../rmg-commands.md#rmggeometryregisterdetectorsfromgdml>.
This only works if a detector mapping exists. `legend-pygeom-tools`
automatically includes such a mapping when using
[`write_pygeom()`](https://legend-pygeom-tools.readthedocs.io/en/stable/api/pygeomtools.html#pygeomtools.write.write_pygeom)
to write the GDML file.

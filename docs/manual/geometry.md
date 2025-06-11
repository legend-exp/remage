(geometry)=

# Experimental geometry

_remage_ does not implement or manage any experimental geometry. Instead, the
user must provide a geometry definition. There are two supported approaches:

- Supply one or more [GDML](https://gdml.web.cern.ch) files via the
  [command line](./running.md) or using the macro command
  <project:../rmg-commands.md#rmggeometryincludegdmlfile>.
- Override {cpp:func}`RMGHardware::DefineGeometry` in a subclass of
  `RMGHardware` and register it using {cpp:func}`RMGManager::SetUserInit`. A
  practical example is in `examples/02-hpge`.

:::{seealso}

The [pyg4ometry](https://pyg4ometry.readthedocs.io) Python library provides
powerful tools for building Monte Carlo geometries and exporting them as GDML.
Relevant pyg4ometry-based packages include:

- [legend-pygeom-tools](https://legend-pygeom-tools.readthedocs.io) – general
  geometry routines and extensions
- [legend-pygeom-hpges](https://legend-pygeom-hpges.readthedocs.io) –
  high-purity germanium detector models
- [legend-pygeom-optics](https://legend-pygeom-optics.readthedocs.io) – optical
  properties for Geant4 materials

Full geometry implementations based on these tools:

- [legend-pygeom-l200](https://github.com/legend-exp/legend-pygeom-l200) –
  LEGEND-200
- [legend-pygeom-l1000](https://github.com/legend-exp/legend-pygeom-l1000) –
  LEGEND-1000

:::

## Registering sensitive detectors

Sensitive detector volumes must be registered so that particle interactions are
recorded in the output. In _remage_, this can be done in several ways. Each
detector has a unique id (`uid`) and a _type_, which determines how hits are
processed and stored.

:::{note}

Custom detector types cannot currently be registered at runtime.

:::

The simplest method is to use the
<project:../rmg-commands.md#rmggeometryregisterdetector> macro command:

```
/RMG/Geometry/RegisterDetector Germanium B00000B 1
/RMG/Geometry/RegisterDetector Germanium C000RG1 2
```

This registers the physical volumes `B00000B` and `C000RG1` as `Germanium`
detectors with `uid`s 1 and 2. If the copy number is not specified, `0` is used
by default. See {ref}`output` for details on how detector types affect output.

Alternatively, detectors can be imported from a GDML file that includes
metadata, using the
<project:../rmg-commands.md#rmggeometryregisterdetectorsfromgdml> command.

:::{tip}

The `legend-pygeom-tools` package automatically includes such metadata when
writing GDML with
[`write_pygeom()`](https://legend-pygeom-tools.readthedocs.io/en/stable/api/pygeomtools.html#pygeomtools.write.write_pygeom).

:::

## Inspecting geometry

To inspect the simulation geometry, use the following macro commands:

- <project:../rmg-commands.md#rmggeometryprintlistoflogicalvolumes>
- <project:../rmg-commands.md#rmggeometryprintlistofphysicalvolumes>

Example session:

```remage
remage> /RMG/Geometry/IncludeGDMLFile geometry.gdml
remage> /run/initialize
G4GDML: Reading 'geometry.gdml'...
G4GDML: Reading definitions...
G4GDML: Reading materials...
G4GDML: Reading solids...
G4GDML: Reading structure...
G4GDML: Reading userinfo...
G4GDML: Reading setup...
G4GDML: Reading 'geometry.gdml' done!
Stripping off GDML names of materials, solids and volumes ...
[Summary -> Checking for overlaps in GDML geometry...
remage> /RMG/Geometry/PrintListOfPhysicalVolumes
[Summary ->  · B00000A  // 0 daugh. // 5.54635 g/cm3  // 488.951 g  // 8.81573 cL  // 1 atm // 293.15 K
[Summary ->  · B00000B  // 0 daugh. // 5.54635 g/cm3  // 700.096 g  // 1.26227 dL  // 1 atm // 293.15 K
...
[Summary ->  · wlsr_ttx // 1 daugh. // 350 mg/cm3 // 1.15983 kg // 3.3138 L   // 1 atm // 293.15 K
[Summary ->  · world    // 1 daugh. // 1e-22 mg/cm3 // 154028 kg // 1.54028e+18 km3 // 3e-18 Pa  // 2.73 K
[Summary ->
[Summary -> Total: 171 volumes
```

:::{tip}

As seen above, the Geant4 overlap checker is enabled by default if GDML input is
provided. It can be disabled with the
<project:../rmg-commands.md#rmggeometrygdmldisableoverlapcheck> command.

:::

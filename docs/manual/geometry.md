(manual-geometry)=

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

(manual-geometry-register-sens-det)=

## Registering sensitive detectors

Sensitive detector volumes must be registered so that particle interactions are
recorded in the output. In _remage_, this can be done in several ways. Each
detector has a unique id (UID) and a _type_, which determines how hits in a
physical volume (or in a group of them) are processed and stored. Detector of
type `Germanium`, `Scintillator` and `Optical` are currently supported, see
{ref}`manual-output` for more details.

:::{note}

User-defined detector types cannot currently be registered at runtime.

:::

The simplest method is to use the
<project:../rmg-commands.md#rmggeometryregisterdetector> macro command:

```geant4
/RMG/Geometry/RegisterDetector Germanium B00000B 1
/RMG/Geometry/RegisterDetector Germanium C000RG1 2 1
```

This registers the physical volume `B00000B`, and the `C000RG1` volume with copy
number `1` as `Germanium` detectors with UIDs 1 and 2 respectively. Because for
`B00000B` no copy number was specified, this will register all `B00000B` named
volumes if there are multiple with different copy numbers. This command now also
accepts regex patterns (respecting the
[default `std::regex_match` grammar](https://en.cppreference.com/w/cpp/regex/ecmascript.html)):

```geant4
/RMG/Geometry/RegisterDetector Germanium B.* 1
```

registers all physical volumes starting with `B`. If there are multiple volumes
matching the pattern, they will all be registered alphabetically under
incrementing UIDs. This means the first alphabetical `B.*` match will be
registered under UID 1, the second match will be registered with UID 2 and so
on. It is therefore the responsibility of the user to make sure that no UID will
be duplicated, which is detected by _remage_ and results in an error.

Alternatively, one might want to assign the same UID to multiple physical
volumes, i.e. as if they constitute a single detector unit. In such a scenario,
there would be no way to distinguish hits from different volumes in the
simulation output (except from the coordinates in post-processing). In this
case, the fifth argument (`allow_uid_reuse`) has to be set to `true`:

```geant4
/RMG/Geometry/RegisterDetector Germanium .*_cu_.* 1 .* true
```

This would register any volume with name matching the regular expression
`.*_cu_.*` under the UID 1. In this case the UIDs will **not** be incremented
for multiple matches.

Last but not least, detectors can be imported from a GDML file that includes
metadata, using the
<project:../rmg-commands.md#rmggeometryregisterdetectorsfromgdml> command.

:::{tip}

The `legend-pygeom-tools` package automatically includes such metadata when
writing GDML with {func}`pygeomtools.write.write_pygeom`.

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

## Checking geometry

_remage_ provides a means to perform additional geometry checks that go beyond
the default overlap checks, by checking the integraity of the volume hierarchy
along a random geantino path. The user has to initialize this test in a
specialized macro:

```geant4
/RMG/Output/ActivateOutputScheme GeometryCheck

/run/initialize

/RMG/Generator/Confine Volume
/RMG/Generator/Confinement/SampleOnSurface
/RMG/Generator/Confinement/FirstSamplingVolume Geometrical

/RMG/Generator/Confinement/Geometrical/AddSolid Box
/RMG/Generator/Confinement/Geometrical/CenterPositionX 0 m
/RMG/Generator/Confinement/Geometrical/CenterPositionY 0 m
/RMG/Generator/Confinement/Geometrical/CenterPositionZ 0 m
/RMG/Generator/Confinement/Geometrical/Box/XLength 10 m
/RMG/Generator/Confinement/Geometrical/Box/YLength 10 m
/RMG/Generator/Confinement/Geometrical/Box/ZLength 10 m

/RMG/Generator/Select GPS
/gps/particle     geantino
/gps/energy       1 MeV
/gps/ang/type     iso

/run/beamOn {n}
```

The box length has to adjusted so that the box lies fully within in the world
volume, but also fully contains all daughter volumes of the world volume,
otherwise the check will not be correctly performed.

Macro commands
===============================

```{contents} Command table
:class: this-will-duplicate-information-and-it-is-still-useful-here
:local:
```

## `/RMG/`


**Sub-directories:**

* `/RMG/Manager/` – General commands for controlling the application
* `/RMG/Processes/` – Commands for controlling physics processes
* `/RMG/Geometry/` – Commands for controlling geometry definitions
* `/RMG/Generator/` – Commands for controlling generators
* `/RMG/Output/` – Commands for controlling the simulation output
* `/RMG/GrabmayrGammaCascades/` – Control Peters gamma cascade model

## `/RMG/Manager/`

General commands for controlling the application


**Sub-directories:**

* `/RMG/Manager/Logging/` – Commands for controlling application logging
* `/RMG/Manager/Randomization/` – Commands for controlling randomization settings

**Commands:**

* `Interactive` – Enable interactive mode
* `PrintProgressModulo` – How many processed events before progress information is displayed

### `/RMG/Manager/Interactive`

Enable interactive mode

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit Idle`

### `/RMG/Manager/PrintProgressModulo`

How many processed events before progress information is displayed

* **Range of parameters** – `n > 0`
* **Parameter** – `n`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

## `/RMG/Manager/Logging/`

Commands for controlling application logging


**Commands:**

* `LogLevel` – Set verbosity level of application log

### `/RMG/Manager/Logging/LogLevel`

Set verbosity level of application log

* **Parameter** – `level`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `debug_event debug detail summary warning error fatal nothing`
* **Allowed states** – `PreInit Idle`

## `/RMG/Manager/Randomization/`

Commands for controlling randomization settings


**Commands:**

* `RandomEngine` – Select the random engine (CLHEP)
* `Seed` – Select the initial seed for randomization (CLHEP::HepRandom::setTheSeed)
* `InternalSeed` – Select the initial seed for randomization by using the internal CLHEP table
* `UseSystemEntropy` – Select a random initial seed from system entropy

### `/RMG/Manager/Randomization/RandomEngine`

Select the random engine (CLHEP)

* **Parameter** – `name`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `JamesRandom RanLux MTwist MixMaxRng`
* **Allowed states** – `PreInit Idle`

### `/RMG/Manager/Randomization/Seed`

Select the initial seed for randomization (CLHEP::HepRandom::setTheSeed)

* **Range of parameters** – `n >= 0`
* **Parameter** – `n`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Manager/Randomization/InternalSeed`

Select the initial seed for randomization by using the internal CLHEP table

* **Range of parameters** – `index >= 0 && index < 430`
* **Parameter** – `index`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Manager/Randomization/UseSystemEntropy`

Select a random initial seed from system entropy

* **Allowed states** – `PreInit Idle`

## `/RMG/Processes/`

Commands for controlling physics processes


**Sub-directories:**

* `/RMG/Processes/Stepping/` – Commands for controlling physics processes

**Commands:**

* `DefaultProductionCut` – Set simulation production cuts, for default region for electrons, positions, and gammas. Note: this does not apply to protons, alphas or generic ions.
* `SensitiveProductionCut` – Set simulation production cuts, for sensitive region for electrons, positions, and gammas. Note: this does not apply to protons, alphas or generic ions.
* `OpticalPhysics` – Add optical processes to the physics list
* `OpticalPhysicsMaxOneWLSPhoton` – Use a custom wavelegth shifting process that produces at maximum one secondary photon.
* `LowEnergyEMPhysics` – Add low energy electromagnetic processes to the physics list
* `HadronicPhysics` – Add hadronic processes to the physics list
* `EnableNeutronThermalScattering` – Use thermal scattering cross sections for neutrons
* `EnableGammaAngularCorrelation` – Set correlated gamma emission flag
* `GammaTwoJMAX` – Set max 2J for sampling of angular correlations
* `StoreICLevelData` – Store e- internal conversion data
* `UseGrabmayrsGammaCascades` – Use custom RMGNeutronCapture to apply Grabmayrs gamma cascades.

### `/RMG/Processes/DefaultProductionCut`

Set simulation production cuts, for default region for electrons, positions, and gammas. Note: this does not apply to protons, alphas or generic ions.

* **Parameter** – `cut`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `mm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Processes/SensitiveProductionCut`

Set simulation production cuts, for sensitive region for electrons, positions, and gammas. Note: this does not apply to protons, alphas or generic ions.

* **Parameter** – `cut`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `mm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Processes/OpticalPhysics`

Add optical processes to the physics list

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit`

### `/RMG/Processes/OpticalPhysicsMaxOneWLSPhoton`

Use a custom wavelegth shifting process that produces at maximum one secondary photon.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit`

### `/RMG/Processes/LowEnergyEMPhysics`

Add low energy electromagnetic processes to the physics list

Uses Livermore by default

* **Parameter** – `arg0`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Default value** – `Livermore`
  * **Candidates** – `Option1 Option2 Option3 Option4 Penelope Livermore LivermorePolarized None`
* **Allowed states** – `PreInit`

### `/RMG/Processes/HadronicPhysics`

Add hadronic processes to the physics list

Uses None by default

* **Parameter** – `arg0`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Default value** – `Shielding`
  * **Candidates** – `QGSP_BIC_HP QGSP_BERT_HP FTFP_BERT_HP Shielding None`
* **Allowed states** – `PreInit`

### `/RMG/Processes/EnableNeutronThermalScattering`

Use thermal scattering cross sections for neutrons

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit`

### `/RMG/Processes/EnableGammaAngularCorrelation`

Set correlated gamma emission flag

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit`

### `/RMG/Processes/GammaTwoJMAX`

Set max 2J for sampling of angular correlations

* **Range of parameters** – `x > 0`
* **Parameter** – `x`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `PreInit`

### `/RMG/Processes/StoreICLevelData`

Store e- internal conversion data

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit`

### `/RMG/Processes/UseGrabmayrsGammaCascades`

Use custom RMGNeutronCapture to apply Grabmayrs gamma cascades.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit`

## `/RMG/Processes/Stepping/`

Commands for controlling physics processes


**Commands:**

* `DaughterNucleusMaxLifetime` – Determines which unstable daughter nuclei will be killed, if they are at rest, depending on their lifetime.
* `SkipTracking` – Immediately discard tracks after primary particle generation. This feature is meant for debugging primary generation.
* `ResetInitialDecayTime` – If the initial step is a radioactive decay, reset the global time of all its secondary tracks to 0.
* `LargeGlobalTimeUncertaintyWarning` – Warn if the global times of tracks get too large to provide the requested time uncertainty.

### `/RMG/Processes/Stepping/DaughterNucleusMaxLifetime`

Determines which unstable daughter nuclei will be killed, if they are at rest, depending on their lifetime.

This applies to the defined lifetime of the nucleus, and not on the sampled actual halflife of the simulated particle.

Set to -1 to disable this feature.

Uses -1 ns  us by default

* **Parameter** – `max_lifetime`
  * **Parameter type** – `d`
  * **Omittable** – `False`
  * **Default value** – `-1`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `us`
  * **Candidates** – `s ms us ns ps min h d y second millisecond microsecond nanosecond picosecond minute hour day year`
* **Allowed states** – `Idle`

### `/RMG/Processes/Stepping/SkipTracking`

Immediately discard tracks after primary particle generation. This feature is meant for debugging primary generation.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Processes/Stepping/ResetInitialDecayTime`

If the initial step is a radioactive decay, reset the global time of all its secondary tracks to 0.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Processes/Stepping/LargeGlobalTimeUncertaintyWarning`

Warn if the global times of tracks get too large to provide the requested time uncertainty.

Uses 1 us by default

* **Parameter** – `value`
  * **Parameter type** – `d`
  * **Omittable** – `False`
  * **Default value** – `1`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `us`
  * **Candidates** – `s ms us ns ps min h d y second millisecond microsecond nanosecond picosecond minute hour day year`
* **Allowed states** – `Idle`

## `/RMG/Geometry/`

Commands for controlling geometry definitions


**Commands:**

* `GDMLDisableOverlapCheck` – Disable the automatic overlap check after loading a GDML file
* `GDMLOverlapCheckNumPoints` – Change the number of points sampled for overlap checks
* `RegisterDetectorsFromGDML` – Register detectors as saved in the GDML auxval structure, as written by pygeomtools.
* `IncludeGDMLFile` – Use GDML file for geometry definition
* `PrintListOfLogicalVolumes` – Print list of defined logical volumes
* `PrintListOfPhysicalVolumes` – Print list of defined physical volumes
* `RegisterDetector` – register a sensitive detector
* `SetMaxStepSize` – Sets maximum step size for a certain detector

### `/RMG/Geometry/GDMLDisableOverlapCheck`

Disable the automatic overlap check after loading a GDML file

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit`

### `/RMG/Geometry/GDMLOverlapCheckNumPoints`

Change the number of points sampled for overlap checks

* **Parameter** – `value`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `PreInit`

### `/RMG/Geometry/RegisterDetectorsFromGDML`

Register detectors as saved in the GDML auxval structure, as written by pygeomtools.

* **Parameter** – `det_type`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `All`
  * **Candidates** – `All Germanium Optical Scintillator`
* **Allowed states** – `PreInit`

### `/RMG/Geometry/IncludeGDMLFile`

Use GDML file for geometry definition

* **Parameter** – `filename`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit`

### `/RMG/Geometry/PrintListOfLogicalVolumes`

Print list of defined logical volumes

* **Allowed states** – `Idle`

### `/RMG/Geometry/PrintListOfPhysicalVolumes`

Print list of defined physical volumes

* **Allowed states** – `Idle`

### `/RMG/Geometry/RegisterDetector`

register a sensitive detector

* **Parameter** – `type`
    – Detector type
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `Germanium Optical Scintillator`
* **Parameter** – `pv_name`
    – Detector physical volume, accepts regex patterns
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Parameter** – `uid`
    – unique detector id
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Parameter** – `copy_nr`
    – copy nr, accepts regex patterns (default 0)
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `0`
* **Parameter** – `allow_id_reuse`
    – append this volume to a previously allocated unique detector id, instead of erroring out.
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `false`
* **Allowed states** – `PreInit`

### `/RMG/Geometry/SetMaxStepSize`

Sets maximum step size for a certain detector

* **Parameter** – `step_size`
  * **Parameter type** – `d`
  * **Omittable** – `False`
  * **Default value** – `1`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Parameter** – `pv_name`
    – Detector physical volume, accepts regex patterns
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit`

## `/RMG/Generator/`

Commands for controlling generators


**Sub-directories:**

* `/RMG/Generator/Confinement/` – Commands for controlling primary confinement
* `/RMG/Generator/MUSUNCosmicMuons/` – Commands for controlling the MUSUN µ generator
* `/RMG/Generator/CosmicMuons/` – Commands for controlling the µ generator
* `/RMG/Generator/BxDecay0/` – Commands for controlling the BxDecay0 generator
* `/RMG/Generator/FromFile/` – Commands for controlling reading event data from file

**Commands:**

* `Confine` – Select primary confinement strategy
* `Select` – Select event generator

### `/RMG/Generator/Confine`

Select primary confinement strategy

* **Parameter** – `strategy`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `UnConfined Volume FromFile`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Select`

Select event generator

* **Parameter** – `generator`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `G4gun GPS BxDecay0 FromFile CosmicMuons MUSUNCosmicMuons UserDefined Undefined`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/Confinement/`

Commands for controlling primary confinement


**Sub-directories:**

* `/RMG/Generator/Confinement/Physical/` – Commands for setting physical volumes up for primary confinement
* `/RMG/Generator/Confinement/Geometrical/` – Commands for setting geometrical volumes up for primary confinement
* `/RMG/Generator/Confinement/FromFile/` – Commands for controlling reading event vertex positions from file

**Commands:**

* `Reset` – Reset all parameters of vertex confinement, so that it can be reconfigured.
* `SampleOnSurface` – If true (or omitted argument), sample on the surface of solids
* `SamplingMode` – Select sampling mode for volume confinement
* `FirstSamplingVolume` – Select the type of volume which will be sampled first for intersections
* `MaxSamplingTrials` – Set maximum number of attempts for sampling primary positions in a volume
* `SurfaceSampleMaxIntersections` – Set maximum number of intersections of a line with the surface. Note: can be set to an overestimate.
* `ForceContainmentCheck` – If true (or omitted argument), perform a containment check even after sampling from a natively sampleable object. This is only an extra sanity check that does not alter the behaviour.

### `/RMG/Generator/Confinement/Reset`

Reset all parameters of vertex confinement, so that it can be reconfigured.

* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/SampleOnSurface`

If true (or omitted argument), sample on the surface of solids

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/SamplingMode`

Select sampling mode for volume confinement

* **Parameter** – `mode`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `IntersectPhysicalWithGeometrical UnionAll SubtractGeometrical`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/FirstSamplingVolume`

Select the type of volume which will be sampled first for intersections

* **Parameter** – `type`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `Physical Geometrical Unset`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/MaxSamplingTrials`

Set maximum number of attempts for sampling primary positions in a volume

* **Range of parameters** – `N > 0`
* **Parameter** – `N`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/SurfaceSampleMaxIntersections`

Set maximum number of intersections of a line with the surface. Note: can be set to an overestimate. 

* **Range of parameters** – `N > 1`
* **Parameter** – `N`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/ForceContainmentCheck`

If true (or omitted argument), perform a containment check even after sampling from a natively sampleable object. This is only an extra sanity check that does not alter the behaviour.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/Confinement/Physical/`

Commands for setting physical volumes up for primary confinement


**Commands:**

* `AddVolume` – Add physical volume(s) to sample primaries from.

### `/RMG/Generator/Confinement/Physical/AddVolume`

Add physical volume(s) to sample primaries from.

* **Parameter** – `regex`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Parameter** – `copy_nr_regex`
  * **Parameter type** – `s`
  * **Omittable** – `True`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/Confinement/Geometrical/`

Commands for setting geometrical volumes up for primary confinement


**Sub-directories:**

* `/RMG/Generator/Confinement/Geometrical/Sphere/` – Commands for setting geometrical dimensions of a sampling sphere
* `/RMG/Generator/Confinement/Geometrical/Cylinder/` – Commands for setting geometrical dimensions of a sampling cylinder
* `/RMG/Generator/Confinement/Geometrical/Box/` – Commands for setting geometrical dimensions of a sampling box

**Commands:**

* `AddSolid` – Add geometrical solid to sample primaries from
* `AddExcludedSolid` – Add geometrical solid to exclude samples from
* `CenterPositionX` – Set center position (X coordinate)
* `CenterPositionY` – Set center position (Y coordinate)
* `CenterPositionZ` – Set center position (Z coordinate)

### `/RMG/Generator/Confinement/Geometrical/AddSolid`

Add geometrical solid to sample primaries from

* **Parameter** – `solid`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `Sphere Cylinder Box`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/AddExcludedSolid`

Add geometrical solid to exclude samples from

* **Parameter** – `solid`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `Sphere Cylinder Box`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/CenterPositionX`

Set center position (X coordinate)

* **Parameter** – `value`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/CenterPositionY`

Set center position (Y coordinate)

* **Parameter** – `value`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/CenterPositionZ`

Set center position (Z coordinate)

* **Parameter** – `value`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/Confinement/Geometrical/Sphere/`

Commands for setting geometrical dimensions of a sampling sphere


**Commands:**

* `InnerRadius` – Set inner radius
* `OuterRadius` – Set outer radius

### `/RMG/Generator/Confinement/Geometrical/Sphere/InnerRadius`

Set inner radius

* **Range of parameters** – `L >= 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/Sphere/OuterRadius`

Set outer radius

* **Range of parameters** – `L > 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/Confinement/Geometrical/Cylinder/`

Commands for setting geometrical dimensions of a sampling cylinder


**Commands:**

* `InnerRadius` – Set inner radius
* `OuterRadius` – Set outer radius
* `Height` – Set height
* `StartingAngle` – Set starting angle
* `SpanningAngle` – Set spanning angle

### `/RMG/Generator/Confinement/Geometrical/Cylinder/InnerRadius`

Set inner radius

* **Range of parameters** – `L >= 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/Cylinder/OuterRadius`

Set outer radius

* **Range of parameters** – `L > 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/Cylinder/Height`

Set height

* **Range of parameters** – `L > 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/Cylinder/StartingAngle`

Set starting angle

* **Parameter** – `A`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/Cylinder/SpanningAngle`

Set spanning angle

* **Parameter** – `A`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/Confinement/Geometrical/Box/`

Commands for setting geometrical dimensions of a sampling box


**Commands:**

* `XLength` – Set X length
* `YLength` – Set Y length
* `ZLength` – Set Z length

### `/RMG/Generator/Confinement/Geometrical/Box/XLength`

Set X length

* **Range of parameters** – `L > 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/Box/YLength`

Set Y length

* **Range of parameters** – `L > 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/Geometrical/Box/ZLength`

Set Z length

* **Range of parameters** – `L > 0`
* **Parameter** – `L`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `cm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/Confinement/FromFile/`

Commands for controlling reading event vertex positions from file


**Commands:**

* `FileName` – Set name of the file containing vertex positions for the next run. See the documentation for a specification of the format.
* `NtupleDirectory` – Change the default input directory/group for ntuples.

### `/RMG/Generator/Confinement/FromFile/FileName`

Set name of the file containing vertex positions for the next run. See the documentation for a specification of the format.

* **Parameter** – `filename`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/Confinement/FromFile/NtupleDirectory`

Change the default input directory/group for ntuples.

:::{note}
this option only has an effect for LH5 or HDF5 input files.
:::

Uses "vtx" by default

* **Parameter** – `nt_directory`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Default value** – `vtx`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/MUSUNCosmicMuons/`

Commands for controlling the MUSUN µ generator


**Commands:**

* `MUSUNFile` – Set the MUSUN input file

### `/RMG/Generator/MUSUNCosmicMuons/MUSUNFile`

Set the MUSUN input file

* **Parameter** – `MUSUNFileName`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/CosmicMuons/`

Commands for controlling the µ generator


**Commands:**

* `SkyShape` – Geometrical shape of the µ generation surface
* `SkyPlaneSize` – Length of the side of the sky, if it has a planar shape
* `SkyPlaneHeight` – Height of the sky, if it has a planar shape
* `MomentumMin` – Minimum momentum of the generated muon
* `MomentumMax` – Maximum momentum of the generated muon
* `ThetaMin` – Minimum azimutal angle of the generated muon momentum
* `ThetaMax` – Maximum azimutal angle of the generated muon momentum
* `PhiMin` – Minimum zenith angle of the generated muon momentum
* `PhiMax` – Maximum zenith angle of the generated muon momentum
* `SpherePositionThetaMin` – Minimum azimutal angle of the generated muon position on the sphere
* `SpherePositionThetaMax` – Maximum azimutal angle of the generated muon position on the sphere
* `SpherePositionPhiMin` – Minimum zenith angle of the generated muon position on the sphere
* `SpherePositionPhiMax` – Maximum zenith angle of the generated muon position on the sphere

### `/RMG/Generator/CosmicMuons/SkyShape`

Geometrical shape of the µ generation surface

* **Parameter** – `shape`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `Plane Sphere`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/SkyPlaneSize`

Length of the side of the sky, if it has a planar shape

* **Range of parameters** – `l > 0`
* **Parameter** – `l`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `m`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/SkyPlaneHeight`

Height of the sky, if it has a planar shape

* **Range of parameters** – `l > 0`
* **Parameter** – `l`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `m`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/MomentumMin`

Minimum momentum of the generated muon

* **Range of parameters** – `p >= 0 && p < 1000`
* **Parameter** – `p`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `GeV/c`
  * **Candidates** – `eV/c keV/c MeV/c GeV/c TeV/c eV/c keV/c MeV/c GeV/c TeV/c`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/MomentumMax`

Maximum momentum of the generated muon

* **Range of parameters** – `p > 0 && p <= 1000`
* **Parameter** – `p`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `GeV/c`
  * **Candidates** – `eV/c keV/c MeV/c GeV/c TeV/c eV/c keV/c MeV/c GeV/c TeV/c`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/ThetaMin`

Minimum azimutal angle of the generated muon momentum

* **Range of parameters** – `a >= 0 && a < 90`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/ThetaMax`

Maximum azimutal angle of the generated muon momentum

* **Range of parameters** – `a > 0 && a <= 90`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/PhiMin`

Minimum zenith angle of the generated muon momentum

* **Range of parameters** – `a >= 0 && a < 360`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/PhiMax`

Maximum zenith angle of the generated muon momentum

* **Range of parameters** – `a > 0 && a <= 360`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/SpherePositionThetaMin`

Minimum azimutal angle of the generated muon position on the sphere

* **Range of parameters** – `a >= 0 && a < 90`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/SpherePositionThetaMax`

Maximum azimutal angle of the generated muon position on the sphere

* **Range of parameters** – `a > 0 && a <= 90`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/SpherePositionPhiMin`

Minimum zenith angle of the generated muon position on the sphere

* **Range of parameters** – `a >= 0 && a < 360`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/CosmicMuons/SpherePositionPhiMax`

Maximum zenith angle of the generated muon position on the sphere

* **Range of parameters** – `a > 0 && a <= 360`
* **Parameter** – `a`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `deg`
  * **Candidates** – `rad mrad deg radian milliradian degree`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/BxDecay0/`

Commands for controlling the BxDecay0 generator


**Commands:**

* `Background` – Set the isotope for the Background mode of the BxDecay0 generator. E.g. 'Co60'
* `DoubleBetaDecay` – Set the isotope, process and energy level for the double beta decay mode of the BxDecay0 generator

### `/RMG/Generator/BxDecay0/Background`

Set the isotope for the Background mode of the BxDecay0 generator. E.g. 'Co60'

* **Parameter** – `isotope`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `Ac228 Am241 Ar39 Ar42 As79+Se79m Bi207+Pb207m Bi208 Bi210 Bi212+Po212 Bi214+Po214 C14 Ca48+Sc48 Cd113 Co60 Cs136 Cs137+Ba137m Eu147 Eu152 Eu154 Gd146 Hf182 I126 I133 I134 I135 K40 K42 Kr81 Kr85 Mn54 Na22 P32 Pa231 Pa234m Pb210 Pb211 Pb212 Pb214 Po210 Po218 Ra226 Ra228 Rb87 Rh106 Rn222 Sb125 Sb126 Sb133 Sr90 Ta180m-B- Ta180m-EC Ta182 Te133 Te133m Te134 Th230 Th234 Tl207 Tl208 U234 U238 Xe129m Xe131m Xe133 Xe135 Y88 Y90 Zn65 Zr96+Nb96`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/BxDecay0/DoubleBetaDecay`

Set the isotope, process and energy level for the double beta decay mode of the BxDecay0 generator

* **Parameter** – `isotope`
    – Set the isotope for the double beta decay
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `Bi214 Ca40 Ca46 Ca48 Cd106 Cd108 Cd114 Cd116 Ce136 Ce138 Ce142 Dy156 Dy158 Er162 Er164 Er170 Ge76 Mo100 Mo92 Nd148 Nd150 Ni58 Os184 Os192 Pb214 Po218 Pt190 Pt198 Rn222 Ru104 Ru96 Se74 Se82 Sm144 Sm154 Sn112 Sn122 Sn124 Sr84 Te120 Te128 Te130 W180 W186 Xe136 Yb168 Yb176 Zn64 Zn70 Zr94 Zr96`
* **Parameter** – `process`
    – Name the decay process you want to simulate
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `0vbb 0vbb_lambda_0 0vbb_lambda_02 2vbb 0vbb_M1 0vbb_M2 0vbb_M3 0vbb_M7 0vbb_lambda_2 2vbb_2 0vkb 2vkb 0v2k 2v2k 2vbb_bos0 2vbb_bos2 0vbb_eta_s 0vbb_eta_nmes 2vbb_lv 0v4b`
* **Parameter** – `level`
    – Energy level of the daughter nucleus
  * **Parameter type** – `i`
  * **Omittable** – `True`
  * **Default value** – `0`
* **Allowed states** – `PreInit Idle`

## `/RMG/Generator/FromFile/`

Commands for controlling reading event data from file


**Commands:**

* `FileName` – Set name of the file containing vertex kinetics for the next run. See the documentation for a specification of the format.
* `NtupleDirectory` – Change the default input directory/group for ntuples.

### `/RMG/Generator/FromFile/FileName`

Set name of the file containing vertex kinetics for the next run. See the documentation for a specification of the format.

* **Parameter** – `filename`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Generator/FromFile/NtupleDirectory`

Change the default input directory/group for ntuples.

:::{note}
this option only has an effect for LH5 or HDF5 input files.
:::

* **Parameter** – `nt_directory`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Default value** – `vtx`
* **Allowed states** – `PreInit Idle`

## `/RMG/Output/`

Commands for controlling the simulation output


**Sub-directories:**

* `/RMG/Output/Germanium/` – Commands for controlling output from hits in germanium detectors.
* `/RMG/Output/Optical/` – Commands for controlling output from hits in optical detectors.
* `/RMG/Output/Vertex/` – Commands for controlling output of primary vertices.
* `/RMG/Output/Scintillator/` – Commands for controlling output from hits in scintillator detectors.
* `/RMG/Output/IsotopeFilter/` – Commands for filtering event out by created isotopes.
* `/RMG/Output/Track/` – Commands for controlling output of track vertices.
* `/RMG/Output/ParticleFilter/` – Commands for filtering particles out by PDG encoding.

**Commands:**

* `FileName` – Set output file name for object persistency
* `NtuplePerDetector` – Create a ntuple for each sensitive detector to store hits. Otherwise, store all hits of one detector type in one ntuple.
* `NtupleUseVolumeName` – Use the sensitive volume name to name output ntuples.
* `ActivateOutputScheme` – Activates the output scheme that had been registered under the given name.
* `NtupleDirectory` – Change the default output directory/group for ntuples in output files.

### `/RMG/Output/FileName`

Set output file name for object persistency

* **Parameter** – `filename`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Output/NtuplePerDetector`

Create a ntuple for each sensitive detector to store hits. Otherwise, store all hits of one detector type in one ntuple.

* **Parameter** – `nt_per_det`
  * **Parameter type** – `b`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Output/NtupleUseVolumeName`

Use the sensitive volume name to name output ntuples.

:::{note}
this only works if `NtuplePerDetector` is set to true.
:::

* **Parameter** – `nt_vol_name`
  * **Parameter type** – `b`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

### `/RMG/Output/ActivateOutputScheme`

Activates the output scheme that had been registered under the given name.

* **Parameter** – `oscheme`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit`

### `/RMG/Output/NtupleDirectory`

Change the default output directory/group for ntuples in output files.

:::{note}
This setting is not respected by all output formats.
:::

* **Parameter** – `nt_directory`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

## `/RMG/Output/Germanium/`

Commands for controlling output from hits in germanium detectors.


**Sub-directories:**

* `/RMG/Output/Germanium/Cluster/` – Commands for controlling clustering of hits in germanium detectors.

**Commands:**

* `EdepCutLow` – Set a lower energy cut that has to be met for this event to be stored.
* `EdepCutHigh` – Set an upper energy cut that has to be met for this event to be stored.
* `AddDetectorForEdepThreshold` – Take this detector into account for the filtering by /EdepThreshold. If this is not set all detectors are used.
* `DiscardPhotonsIfNoGermaniumEdep` – Discard optical photons (before simulating them), if no edep in germanium detectors occurred in the same event.
* `StoreSinglePrecisionPosition` – Use float32 (instead of float64) for position output.
* `StoreSinglePrecisionEnergy` – Use float32 (instead of float64) for energy output.
* `DiscardZeroEnergyHits` – Discard hits with zero energy.
* `StoreTrackID` – Store Track IDs for hits in the output file.
* `StepPositionMode` – Select which position of the step to store

### `/RMG/Output/Germanium/EdepCutLow`

Set a lower energy cut that has to be met for this event to be stored.

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `keV`
  * **Candidates** – `eV keV MeV GeV TeV PeV meV J electronvolt kiloelectronvolt megaelectronvolt gigaelectronvolt teraelectronvolt petaelectronvolt millielectronVolt joule`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/EdepCutHigh`

Set an upper energy cut that has to be met for this event to be stored.

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `keV`
  * **Candidates** – `eV keV MeV GeV TeV PeV meV J electronvolt kiloelectronvolt megaelectronvolt gigaelectronvolt teraelectronvolt petaelectronvolt millielectronVolt joule`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/AddDetectorForEdepThreshold`

Take this detector into account for the filtering by /EdepThreshold. If this is not set all detectors are used.

* **Parameter** – `det_uid`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/DiscardPhotonsIfNoGermaniumEdep`

Discard optical photons (before simulating them), if no edep in germanium detectors occurred in the same event.

:::{note}
If another output scheme also requests the photons to be discarded, the germanium edep filter does not force the photons to be simulated.
:::

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/StoreSinglePrecisionPosition`

Use float32 (instead of float64) for position output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/StoreSinglePrecisionEnergy`

Use float32 (instead of float64) for energy output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/DiscardZeroEnergyHits`

Discard hits with zero energy.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/StoreTrackID`

Store Track IDs for hits in the output file.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/StepPositionMode`

Select which position of the step to store

Uses Average by default

* **Parameter** – `mode`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `PreStep PostStep Average Both`
* **Allowed states** – `Idle`

## `/RMG/Output/Germanium/Cluster/`

Commands for controlling clustering of hits in germanium detectors.


**Commands:**

* `PreClusterOutputs` – Pre-Cluster output hits before saving
* `CombineLowEnergyElectronTracks` – Merge low energy electron tracks.
* `RedistributeGammaEnergy` – Redistribute energy deposited by gamma tracks to nearby electron tracks.
* `PreClusterDistance` – Set a distance threshold for the bulk pre-clustering.
* `PreClusterDistanceSurface * Uses 0 fm` – by default
* `PreClusterTimeThreshold` – Set a time threshold for pre-clustering.
* `SurfaceThickness` – Set a surface thickness for the Germanium detector.
* `ElectronTrackEnergyThreshold` – Set a energy threshold for tracks to be merged.

### `/RMG/Output/Germanium/Cluster/PreClusterOutputs`

Pre-Cluster output hits before saving

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/Cluster/CombineLowEnergyElectronTracks`

Merge low energy electron tracks.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/Cluster/RedistributeGammaEnergy`

Redistribute energy deposited by gamma tracks to nearby electron tracks.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/Cluster/PreClusterDistance`

Set a distance threshold for the bulk pre-clustering.

Uses 50 um  by default

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `um`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/Cluster/PreClusterDistanceSurface`

Uses 0 fm  by default

Set a distance threshold for the surface pre-clustering.

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `um`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/Cluster/PreClusterTimeThreshold`

Set a time threshold for pre-clustering.

Uses 10 us  by default

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `us`
  * **Candidates** – `s ms us ns ps min h d y second millisecond microsecond nanosecond picosecond minute hour day year`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/Cluster/SurfaceThickness`

Set a surface thickness for the Germanium detector.

Uses 2 mm  by default

* **Parameter** – `thickness`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `mm`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `Idle`

### `/RMG/Output/Germanium/Cluster/ElectronTrackEnergyThreshold`

Set a energy threshold for tracks to be merged.

Uses 10 keV by default

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `keV`
  * **Candidates** – `eV keV MeV GeV TeV PeV meV J electronvolt kiloelectronvolt megaelectronvolt gigaelectronvolt teraelectronvolt petaelectronvolt millielectronVolt joule`
* **Allowed states** – `Idle`

## `/RMG/Output/Optical/`

Commands for controlling output from hits in optical detectors.


**Commands:**

* `StoreSinglePrecisionEnergy` – Use float32 (instead of float64) for wavelength output.

### `/RMG/Output/Optical/StoreSinglePrecisionEnergy`

Use float32 (instead of float64) for wavelength output.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

## `/RMG/Output/Vertex/`

Commands for controlling output of primary vertices.


**Commands:**

* `StorePrimaryParticleInformation` – Store information on primary particle details (not only vertex data).
* `SkipPrimaryVertexOutput` – Do not store vertex/primary particle data.
* `StoreSinglePrecisionPosition` – Use float32 (instead of float64) for position output.
* `StoreSinglePrecisionEnergy` – Use float32 (instead of float64) for energy output.

### `/RMG/Output/Vertex/StorePrimaryParticleInformation`

Store information on primary particle details (not only vertex data).

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Vertex/SkipPrimaryVertexOutput`

Do not store vertex/primary particle data.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Vertex/StoreSinglePrecisionPosition`

Use float32 (instead of float64) for position output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Vertex/StoreSinglePrecisionEnergy`

Use float32 (instead of float64) for energy output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

## `/RMG/Output/Scintillator/`

Commands for controlling output from hits in scintillator detectors.


**Sub-directories:**

* `/RMG/Output/Scintillator/Cluster/` – Commands for controlling clustering of hits in scintillator detectors.

**Commands:**

* `EdepCutLow` – Set a lower energy cut that has to be met for this event to be stored.
* `EdepCutHigh` – Set an upper energy cut that has to be met for this event to be stored.
* `AddDetectorForEdepThreshold` – Take this detector into account for the filtering by /EdepThreshold.
* `DiscardZeroEnergyHits` – Discard hits with zero energy.
* `StoreParticleVelocities` – Store velocities of particle in the output file.
* `StoreTrackID` – Store Track IDs for hits in the output file.
* `StoreSinglePrecisionPosition` – Use float32 (instead of float64) for position output.
* `StoreSinglePrecisionEnergy` – Use float32 (instead of float64) for energy output.
* `StepPositionMode` – Select which position of the step to store

### `/RMG/Output/Scintillator/EdepCutLow`

Set a lower energy cut that has to be met for this event to be stored.

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `keV`
  * **Candidates** – `eV keV MeV GeV TeV PeV meV J electronvolt kiloelectronvolt megaelectronvolt gigaelectronvolt teraelectronvolt petaelectronvolt millielectronVolt joule`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/EdepCutHigh`

Set an upper energy cut that has to be met for this event to be stored.

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `keV`
  * **Candidates** – `eV keV MeV GeV TeV PeV meV J electronvolt kiloelectronvolt megaelectronvolt gigaelectronvolt teraelectronvolt petaelectronvolt millielectronVolt joule`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/AddDetectorForEdepThreshold`

Take this detector into account for the filtering by /EdepThreshold.

* **Parameter** – `det_uid`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/DiscardZeroEnergyHits`

Discard hits with zero energy.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/StoreParticleVelocities`

Store velocities of particle in the output file.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/StoreTrackID`

Store Track IDs for hits in the output file.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/StoreSinglePrecisionPosition`

Use float32 (instead of float64) for position output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/StoreSinglePrecisionEnergy`

Use float32 (instead of float64) for energy output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/StepPositionMode`

Select which position of the step to store

* **Parameter** – `mode`
  * **Parameter type** – `s`
  * **Omittable** – `False`
  * **Candidates** – `PreStep PostStep Average Both`
* **Allowed states** – `Idle`

## `/RMG/Output/Scintillator/Cluster/`

Commands for controlling clustering of hits in scintillator detectors.


**Commands:**

* `PreClusterOutputs` – Pre-Cluster output hits before saving
* `CombineLowEnergyElectronTracks` – Merge low energy electron tracks.
* `RedistributeGammaEnergy` – Redistribute energy deposited by gamma tracks to nearby electron tracks.
* `PreClusterDistance` – Set a distance threshold for the bulk pre-clustering.
* `PreClusterTimeThreshold` – Set a time threshold for pre-clustering.
* `ElectronTrackEnergyThreshold` – Set a energy threshold for tracks to be merged.

### `/RMG/Output/Scintillator/Cluster/PreClusterOutputs`

Pre-Cluster output hits before saving

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/Cluster/CombineLowEnergyElectronTracks`

Merge low energy electron tracks.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/Cluster/RedistributeGammaEnergy`

Redistribute energy deposited by gamma tracks to nearby electron tracks.

This is enabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/Cluster/PreClusterDistance`

Set a distance threshold for the bulk pre-clustering.

Uses 500 um  by default

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `um`
  * **Candidates** – `pc km m cm mm um nm Ang fm parsec kilometer meter centimeter millimeter micrometer nanometer angstrom fermi`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/Cluster/PreClusterTimeThreshold`

Set a time threshold for pre-clustering.

Uses 10 us  by default

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `us`
  * **Candidates** – `s ms us ns ps min h d y second millisecond microsecond nanosecond picosecond minute hour day year`
* **Allowed states** – `Idle`

### `/RMG/Output/Scintillator/Cluster/ElectronTrackEnergyThreshold`

Set a energy threshold for tracks to be merged.

Uses 10 keV by default

* **Parameter** – `threshold`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Parameter** – `Unit`
  * **Parameter type** – `s`
  * **Omittable** – `True`
  * **Default value** – `keV`
  * **Candidates** – `eV keV MeV GeV TeV PeV meV J electronvolt kiloelectronvolt megaelectronvolt gigaelectronvolt teraelectronvolt petaelectronvolt millielectronVolt joule`
* **Allowed states** – `Idle`

## `/RMG/Output/IsotopeFilter/`

Commands for filtering event out by created isotopes.


**Commands:**

* `AddIsotope` – Add an isotope to the list. Only events that have a track with this isotope at any point in time will be persisted.
* `DiscardPhotonsIfIsotopeNotProduced` – Discard optical photons (before simulating them), if the specified isotopes had not been produced in the same event.

### `/RMG/Output/IsotopeFilter/AddIsotope`

Add an isotope to the list. Only events that have a track with this isotope at any point in time will be persisted.

* **Parameter** – `A`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Parameter** – `Z`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/IsotopeFilter/DiscardPhotonsIfIsotopeNotProduced`

Discard optical photons (before simulating them), if the specified isotopes had not been produced in the same event.

:::{note}
If another output scheme also requests the photons to be discarded, the isotope filter does not force the photons to be simulated.
:::

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

## `/RMG/Output/Track/`

Commands for controlling output of track vertices.


**Commands:**

* `AddProcessFilter` – Only include tracks created by this process.
* `AddParticleFilter` – Only include tracks with this particle.
* `EnergyFilter` – Only include tracks with kinetic energy above this threshold.
* `StoreSinglePrecisionPosition` – Use float32 (instead of float64) for position output.
* `StoreSinglePrecisionEnergy` – Use float32 (instead of float64) for energy output.
* `StoreAlways` – Always store track data, even if event should be discarded.

### `/RMG/Output/Track/AddProcessFilter`

Only include tracks created by this process.

* **Parameter** – `process`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/Track/AddParticleFilter`

Only include tracks with this particle.

* **Parameter** – `pdgid`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/Track/EnergyFilter`

Only include tracks with kinetic energy above this threshold.

* **Parameter** – `energy`
  * **Parameter type** – `d`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/Track/StoreSinglePrecisionPosition`

Use float32 (instead of float64) for position output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Track/StoreSinglePrecisionEnergy`

Use float32 (instead of float64) for energy output.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

### `/RMG/Output/Track/StoreAlways`

Always store track data, even if event should be discarded.

This is disabled by default

* **Parameter** – `boolean`
  * **Parameter type** – `b`
  * **Omittable** – `True`
  * **Default value** – `true`
* **Allowed states** – `Idle`

## `/RMG/Output/ParticleFilter/`

Commands for filtering particles out by PDG encoding.


**Commands:**

* `AddParticle` – Add a particle to be filtered out by its PDG code. User is responsible for correct PDG code.
* `AddKeepVolume` – Add a physical volume by name in which all specified Particles will be kept. They will be killed everywhere else. Can NOT be mixed with KillVolumes.
* `AddKillVolume` – Add a physical volume by name in which all specified Particles will be killed. They will only be killed in this volume. Can NOT be mixed with KeepVolumes.
* `AddKillProcess` – Add a physics process by name. This will only kill the specified particles when they were created by this process. Can NOT be mixed with KeepProcess.
* `AddKeepProcess` – Add a physics process by name. This will only keep the specified particles when they were created by this process, all other particles will not be kept. Can NOT be mixed with KillProcess.

### `/RMG/Output/ParticleFilter/AddParticle`

Add a particle to be filtered out by its PDG code. User is responsible for correct PDG code.

* **Parameter** – `PDGcode`
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/ParticleFilter/AddKeepVolume`

Add a physical volume by name in which all specified Particles will be kept. They will be killed everywhere else. Can NOT be mixed with KillVolumes.

* **Parameter** – `PhysicalVolumeName`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/ParticleFilter/AddKillVolume`

Add a physical volume by name in which all specified Particles will be killed. They will only be killed in this volume. Can NOT be mixed with KeepVolumes.

* **Parameter** – `PhysicalVolumeName`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/ParticleFilter/AddKillProcess`

Add a physics process by name. This will only kill the specified particles when they were created by this process. Can NOT be mixed with KeepProcess.

* **Parameter** – `proc`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

### `/RMG/Output/ParticleFilter/AddKeepProcess`

Add a physics process by name. This will only keep the specified particles when they were created by this process, all other particles will not be kept. Can NOT be mixed with KillProcess.

* **Parameter** – `proc`
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `Idle`

## `/RMG/GrabmayrGammaCascades/`

Control Peters gamma cascade model


**Commands:**

* `SetGammaCascadeRandomStartLocation` – Set the whether the start location in the gamma cascade file is random or not
* `SetGammaCascadeFile` – Set a gamma cascade file for neutron capture on a specified isotope

### `/RMG/GrabmayrGammaCascades/SetGammaCascadeRandomStartLocation`

Set the whether the start location in the gamma cascade file is random or not

0 = don't

1 = do

* **Parameter** – `arg0`
  * **Parameter type** – `i`
  * **Omittable** – `False`
  * **Default value** – `0`
  * **Candidates** – `0 1`
* **Allowed states** – `PreInit Idle`

### `/RMG/GrabmayrGammaCascades/SetGammaCascadeFile`

Set a gamma cascade file for neutron capture on a specified isotope

* **Parameter** – `Z`
    – Z of isotope
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Parameter** – `A`
    – A of isotope
  * **Parameter type** – `i`
  * **Omittable** – `False`
* **Parameter** – `file`
    – /path/to/file of gamma cascade
  * **Parameter type** – `s`
  * **Omittable** – `False`
* **Allowed states** – `PreInit Idle`

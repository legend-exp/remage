(manual-staging)=

# Staging and waiting stack

This page provides an introduction to the staging options in _remage_. In this
context, "staging" refers to deferring selected tracks from immediate processing
to the waiting stack, which is then handled in a later stage based on
configurable conditions. This can help manage computational load in scenarios
with many low-energy secondaries or optical photons. For complete command
signatures, see the command reference in <project:../rmg-commands.md>.

## Overview

### Motivation

The motivation for deferring certain tracks is to reduce immediate processing
load in configurations with many particles (for example optical physics or muon
showers). In the current implementation, optical photons and secondary electrons
can be staged. Optical photons can be deferred directly, while electrons require
additional physics considerations, described in
[Caveats regarding the electron staging scheme](#caveats-regarding-the-electron-staging-scheme).

### Implementation details

The staging backend in _remage_ is based on Geant4 stacking actions. The
`RMGStagingScheme` class implements `StackingActionClassify`, which assigns
tracks to `fWaiting` or `fUrgent`.

- `fUrgent` tracks are processed immediately.
- `fWaiting` tracks are deferred and revisited in the next stage.

After the urgent stack is exhausted, Geant4 transitions to a new stage. At this
point, `StackingActionNewStage` implementations from other active output/filter
schemes (for example _Germanium_ and _IsotopeFilter_) can decide whether waiting
tracks should be kept or cleared for that event.

### Caveats regarding the electron staging scheme

The electron staging scheme is more complex than the optical-photon staging due
to their usual higher energy.

If stage transition conditions depend on energy deposition in specific volumes
(for example `Germanium`), deferring electrons can move part of the energy
deposit in the conditioned volume (for example Bremsstrahlung-induced gammas) to
a later stage. This can make a stage-transition condition fail in the initial
stage and discard the waiting stack. To mitigate this, use `VolumeSafety` to
defer only electrons sufficiently far from relevant detector surfaces. The best
value depends on geometry and energy, but `20 cm` is often a conservative
starting point for gamma sources.

If stage transition conditions depend on isotope production (for example
`IsotopeFilter`), deferring high-energy electrons in showers may suppress
relevant production channels in the initial stage. To mitigate this, use
`MaxEnergyThresholdForStacking` so only lower-energy electrons are deferred. The
threshold should be tuned for the isotope and reaction channels of interest.

Because of these caveats, use electron staging only after validation the setup
for the physics observables of interest. In some cases, it may be preferable to
stage only optical photons and not electrons, for example, with close-by
sources.

### Recommended usecases

Optical photon staging can always be beneficial compared to no staging with
optical physics. However, there seems to be a bottle neck when putting too many
particles onto the waiting stack. It seems it takes about ~10µs per track to be
put on the waiting stack, and in cases with 10k optical photons, this can lead
to a significant slowdown per event. In addition, in muon simulations, one can
expect 1e8 optical photons per event, which can exhaust the available memory and
crash the simulation.

In these cases, electron staging can be beneficial. Due to the smaller number of
electrons, the positioning on the waiting stack is not a bottleneck, and one can
get significant performance improvements. However, considering the caveats
described above, it is important to validate the setup for the physics
observables of interest. However, this improvement is only relevant when the
fraction of events skipping the waiting stack is small (<1%).

**Therefore, electron staging is recommended for cases with potentially many
optical photons, such as far-away radiogenic, and cosmogenic simulations. In
other cases, such as close-by sources, it may be preferable to stage only
optical photons**.

## Commands

### Optical-photon staging

- `/RMG/Staging/OpticalPhotons/DeferToWaitingStage` enables deferral of optical
  photons to the waiting stack during stage 0.

### Electron staging

- `/RMG/Staging/Electrons/DeferToWaitingStage` enables deferral of secondary
  electrons **as well as optical photons**.
- `/RMG/Staging/Electrons/VolumeSafety` sets a minimum distance-to-surface
  condition.
- `/RMG/Staging/Electrons/MaxEnergyThresholdForStacking` limits deferred
  electrons by kinetic energy.
- `/RMG/Staging/Electrons/AddVolumeName` restricts deferral to named logical
  volumes.
- `/RMG/Staging/Electrons/DistanceCheckGermaniumOnly` enables Germanium-only
  filtering for electron distance checks.

### Stage transition conditions

You can configure conditions that clear waiting tracks at stage transition.
These are separate from the defer-to-waiting staging commands.

- **Germanium condition:**
  `/RMG/Output/Germanium/DiscardWaitingTracksUnlessGermaniumEdep` clears waiting
  tracks unless Germanium energy deposition occurred in the event.
- **IsotopeFilter condition:**
  `/RMG/Output/IsotopeFilter/DiscardWaitingTracksUnlessIsotopeProduced` clears
  waiting tracks unless one of the configured isotopes was produced. Typical
  setup also requires: `/RMG/Output/ActivateOutputScheme` with `IsotopeFilter`
  and at least one `/RMG/Output/IsotopeFilter/AddIsotope` command.

## Configuration checklist

1. Activate required optional output schemes (required: `Staging`, stage
   transition conditions: e.g., `IsotopeFilter`, or `Germanium`).
2. Specify which particles to defer to the waiting stack (e.g., optical photons,
   electrons) with the `Staging` commands.
3. Define the conditions for stage transition (e.g., energy deposition in
   Germanium, isotope production) with the relevant output scheme or filter
   commands.
4. (Optional) Tune the staging parameters for electrons (e.g., safety distance,
   energy threshold) based on the physics of the problem and the geometry.

## Examples

The examples below are intended as a skeleton. Replace placeholders with your
actual geometry volume names, isotopes, and source definitions.

### Gamma example 1: optical staging with Germanium skip condition

```geant4
/RMG/Output/ActivateOutputScheme Staging
/RMG/Geometry/RegisterDetector Germanium detector_phys 0

/run/initialize

/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Output/Germanium/DiscardWaitingTracksUnlessGermaniumEdep true
/RMG/Output/Germanium/EdepCutLow 25 keV

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/particle gamma
/gps/energy 2.6 MeV
/gps/ang/type iso

/run/beamOn 1000
```

### Gamma example 2: optical staging with IsotopeFilter skip condition

```geant4
/RMG/Output/ActivateOutputScheme Staging
/RMG/Output/ActivateOutputScheme IsotopeFilter

/run/initialize

/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Output/IsotopeFilter/AddIsotope 41 18
/RMG/Output/IsotopeFilter/DiscardWaitingTracksUnlessIsotopeProduced true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/particle neutron
/gps/energy 0.025 eV
/gps/direction 0 0 1

/run/beamOn 1000
```

### Electron and optical staging together

```geant4
/RMG/Output/ActivateOutputScheme Staging
/RMG/Geometry/RegisterDetector Germanium detector_phys 0

/run/initialize

/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/VolumeSafety 5.0 cm
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 10.0 MeV
/RMG/Staging/Electrons/AddVolumeName world_vol

/RMG/Output/Germanium/DiscardWaitingTracksUnlessGermaniumEdep true
/RMG/Output/Germanium/EdepCutLow 25 keV

/RMG/Generator/Select GPS
/gps/particle gamma
/gps/energy 2.6 MeV

/run/beamOn 1000
```

### Muon example with electron and optical staging (8 MeV threshold)

The value `8.0 MeV` is a rough estimate for the neutron separation energy of the
isotopes in liquid argon. Below this, no new neutrons are emitted, therefore,
electrons produced below this threshold cannot contribute to isotope production
and can be safely deferred without risking missing isotope production.

```geant4
/RMG/Output/ActivateOutputScheme Staging
/RMG/Output/ActivateOutputScheme IsotopeFilter

/run/initialize

/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/VolumeSafety 20.0 cm
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 8.0 MeV
/RMG/Staging/Electrons/AddVolumeName world_vol

/RMG/Output/IsotopeFilter/AddIsotope 77 32
/RMG/Output/IsotopeFilter/DiscardWaitingTracksUnlessIsotopeProduced true

/RMG/Generator/Select GPS
/gps/particle mu-
/gps/energy 273 GeV
/gps/ang/type iso

/run/beamOn 500
```

## See also

- {ref}`manual-output`
- <project:../rmg-commands.md#rmgstaging>
- <project:../rmg-commands.md#rmgoutputgermanium>
- <project:../rmg-commands.md#rmgoutputisotopefilter>

(manual-staging)=

# Staging and suspending tracks

This page provides an introduction to the staging and suspension options in
_remage_. In this context, "staging" refers to deferring selected tracks from
immediate processing to the waiting stack, which is then handled in a later
stage based on configurable conditions. "Suspending", conversely, refers to
halting and deferring the further processing of tracks that meet certain
criteria. These mechanisms can help manage computational load in scenarios with
many low-energy secondaries or optical photons. For complete command signatures,
see the command reference in <project:../rmg-commands.md>.

## Overview

### Motivation

The motivation for deferring certain tracks is to reduce immediate processing
load in configurations with many particles (for example optical physics or muon
showers). Staging relates to track classification at creation, i.e., whether a
track should be simulated immediately or placed on a waiting stack for later
processing. Suspension relates to tracks that are already being processed and
are then halted and moved to the waiting stack under certain conditions.

At the moment, _remage_ supports staging and suspension for:

- optical photons,
- secondary electrons,
- secondary gammas.

Staging of optical photons is a common use case, since they are at the end of
the physics simulation chain and do not carry energy that would typically alter
the stage-transition conditions discussed below. Deferring their calculation to
a later stage until certain conditions are met can significantly reduce
computational load in scenarios with many optical photons.

Staging and suspension of secondary electrons and gammas are more
physics-sensitive, since these particles can carry significant energy. However,
there are two scenarios where this can be beneficial:

1. In radiogenic simulations of far-away sources with the focus on energy
   deposition in a detector, where many low-energetic electrons are produced far
   away from the detectors and can be safely deferred. To avoid deferring
   electrons close to the detectors, a safety distance condition can be applied,
   which requires electrons to be a minimum distance away from any volume
   surface to be deferred.
2. In muon-showers where the main interest is production of isotopes, where
   electrons and gammas below the threshold of isotope production can be
   deferred. This energy threshold can be tuned by the user.

### Implementation details

The staging and suspension backend in _remage_ is based on Geant4 stacking
actions.

Staging is implemented in the `RMGStagingScheme` class, which implements
`StackingActionClassify` and assigns tracks to `fWaiting` or `fUrgent`.

- `fUrgent` tracks are processed immediately.
- `fWaiting` tracks are deferred and revisited in the next stage.

After the urgent stack is exhausted, Geant4 transitions to a new stage. At this
point, `StackingActionNewStage` implementations from other active output/filter
schemes (for example _Germanium_ and _IsotopeFilter_) can decide whether waiting
tracks should be kept or cleared for that event.

Suspension is implemented separately from track-initialization classification.
`RMGStagingScheme` implements a stepping action hook (`SteppingAction`) for
optional suspension of secondary electrons and gammas when they cross from above
to below a configured energy threshold. This is implemented by calling
`SetTrackStatus(fSuspend)` on the track, which halts processing and places the
track on the waiting stack.

### Caveats regarding electron and gamma staging

Electron and gamma staging are more physics-sensitive than optical-only staging
because they can carry significant energy in electromagnetic cascades.

If stage transition conditions depend on energy deposition in specific volumes
(for example `Germanium`), deferring electrons or gammas can move part of the
energy deposit in the conditioned volume to a later stage. This can make a
stage-transition condition fail in the initial stage and discard the waiting
stack. Gamma staging is not recommended in such cases. Electron staging,
however, can still be viable if a safety-distance condition is applied so that
only tracks far from volume surfaces are deferred. To do this, use
`VolumeSafety` and `AddVolumeName` to defer tracks only in controlled regions.
The best values depend on geometry and source type, but `20 cm` is often a
conservative starting point for far-away gamma sources.

If stage transition conditions depend on isotope production (for example
`IsotopeFilter`), deferring high-energy shower components may suppress relevant
production channels in the initial stage. To mitigate this, use
`MaxEnergyThresholdForStacking` so only lower-energy tracks are deferred. The
threshold should be tuned for the isotope and reaction channels of interest.
This threshold applies to both staging and suspension.

Because of these caveats, validate staging and suspension against the physics
observables of interest. In some cases (for example close-by sources), optical
staging alone may be preferable to avoid the risk of biasing physics
observables.

### Recommended use cases

Optical photon staging can always be beneficial compared to no staging with
optical physics. However, there can be a bottleneck when putting too many
particles onto the waiting stack. It seems it takes about ~10µs per track to be
put on the waiting stack, and in cases with 10k optical photons, this can lead
to a significant slowdown per event. In addition, in muon simulations, one can
expect 1e8 optical photons per event, which can exhaust the available memory and
crash the simulation.

In these cases, electron staging can be beneficial. Due to the smaller number of
electrons, the positioning on the waiting stack is not a bottleneck, and one can
get significant performance improvements. However, considering the caveats
described above, it is important to validate the setup for the physics
observables of interest. This improvement is only relevant when the fraction of
events skipping the waiting stack is small (<1%). Examples of speedup can be
found in the validation section of the documentation.

**Therefore**, electron staging is recommended for cases with potentially many
optical photons, such as far-away radiogenic, and cosmogenic simulations. In
other cases, such as close-by sources, it may be preferable to stage only
optical photons.

Gamma staging is only recommended in muon simulations to further reduce the
sub-production-threshold particle population in showers.

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
- `/RMG/Staging/Electrons/SuspendOnEnergyDrop` enables stepping-time suspension
  when a track crosses from above to below the configured threshold.

### Gamma staging

- `/RMG/Staging/Gammas/DeferToWaitingStage` enables deferral of secondary
  gammas.
- `/RMG/Staging/Gammas/VolumeSafety` sets a minimum distance-to-surface
  condition.
- `/RMG/Staging/Gammas/MaxEnergyThresholdForStacking` limits deferred gammas by
  kinetic energy.
- `/RMG/Staging/Gammas/AddVolumeName` restricts deferral to named logical
  volumes.
- `/RMG/Staging/Gammas/SuspendOnEnergyDrop` enables stepping-time suspension
  when a track crosses from above to below the configured threshold.

### Suspension behavior

- Suspension is evaluated in stepping, not at end-of-track.
- The threshold is reused from `MaxEnergyThresholdForStacking` for the
  corresponding particle class.
- Suspension is applied only to secondary tracks.

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
   electrons, gammas) with the `Staging` commands.
3. Define the conditions for stage transition (e.g., energy deposition in
   Germanium, isotope production) with the relevant output scheme or filter
   commands.
4. (Optional) Enable suspension for electrons and/or gammas.
5. (Optional) Tune safety distances and energy thresholds based on source,
   geometry, and physics observables.

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
and can be safely deferred without risking missed isotope production. Optional
suspension can further reduce the cost of low-energy particles by deferring
tracks after they cross below the configured energy threshold.

```geant4
/RMG/Output/ActivateOutputScheme Staging
/RMG/Output/ActivateOutputScheme IsotopeFilter

/run/initialize

/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 8.0 MeV
/RMG/Staging/Electrons/SuspendOnEnergyDrop true
/RMG/Staging/Electrons/AddVolumeName world_vol

/RMG/Staging/Gammas/DeferToWaitingStage true
/RMG/Staging/Gammas/MaxEnergyThresholdForStacking 8.0 MeV
/RMG/Staging/Gammas/SuspendOnEnergyDrop true
/RMG/Staging/Gammas/AddVolumeName world_vol

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

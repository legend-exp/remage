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

- optical photons (only staging),
- secondary electrons,
- secondary positrons (optional, reusing the electron staging conditions).

Staging of optical photons is a common use case, since they are at the end of
the physics simulation chain and do not carry energy that would typically alter
the stage-transition conditions discussed below. Deferring their calculation to
a later stage until certain conditions are met can significantly reduce
computational load in scenarios with many optical photons.

Staging and suspension of secondary electrons are more physics-sensitive, since
these particles can carry significant energy. However, there are two scenarios
where this can be beneficial:

1. In radiogenic simulations of far-away sources with the focus on energy
   deposition in a detector, where many low-energetic electrons are produced far
   away from the detectors and can be safely deferred. To avoid deferring
   electrons close to the detectors, a safety distance condition can be applied,
   which requires electrons to be a minimum distance away from any Germanium
   detector surface to be deferred.
2. In muon-showers where the main interest is production of isotopes, where
   electrons below the threshold of isotope production can be deferred. This
   energy threshold can be tuned by the user.

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

How the deferred tracks are held between stage 0 and their re-injection is
configurable per particle class. By default they remain on the native Geant4
waiting stack as full `G4Track` objects, which is faithful but memory-heavy:
staging the ~1e8 optical photons of a muon shower this way can exhaust the
available RAM. Enabling `RMGDeferring` switches to a compact custom backend that
instead records each deferred track into a small fixed-size struct (52 bytes per
optical photon, 44 bytes per electron), kills the original, and rebuilds and
re-injects an equivalent track later, requiring roughly an order of magnitude
less memory per staged track. The record holds the track state (position,
momentum direction, kinetic energy, global time, polarization for photons, plus
the statistical weight (biasing) and parent id) in single precision, so a
re-injected track reproduces the original to float32 precision (~1e-7 relative).
Everything else, like the `CreatorProcess` or the `TouchableHistory` is not
propagated to the new track.

:::{warning}

As the `CreatorProcess` is not propagated to the new track, filtering track
output by process (with
<project:../rmg-commands.md#rmgoutputtrackaddprocessfilter>) will not work on
these tracks, and (if enabled) will discard all track output rows of re-injected
tracks.

:::

`LimitMemory` bounds the memory the _rebuilt_ tracks may occupy: at each stage
transition the custom backend re-injects at most `LimitMemory` megabytes worth
of full `G4Track`s (sized from the actual Geant4 track footprint), spreading a
large staged population over several stages so the reconstituted stack never
grows without bound. This applies whether or not the records are spilled to
disk.

Setting `StorePath` to a directory additionally caps the _recording_ side: the
compact record buffer is flushed to a scratch file whenever it exceeds
`LimitMemory` megabytes, so the resident records also stay bounded no matter how
many tracks are staged. Without `StorePath`, the record buffer instead grows in
RAM (each record is tiny, but the buffer itself is uncapped) and only the
re-injection limit above applies. The scratch files are unlinked as soon as they
are created, so they never outlive the run however it ends; point `StorePath` at
a real disk, since a `tmpfs` mount such as `/tmp` or `/dev/shm` would spill back
into RAM and defeat the cap. _remage_ warns at startup if the scratch directory
has less than 10 GB free. If the scratch directory runs out of memory during the
run, _remage_ will fatal and the (already unlinked) scratch files will
disappear, freeing the space again. This guards against accidentally filling
your disk, potentially preventing the system to boot again.

These caps apply per event, not per run. At the start of every event _remage_
clears the in-memory record buffers and truncates the scratch files back to
zero, so each thread only ever holds the records of the event it is currently
processing. Nothing accumulates across events: the memory and scratch space are
reused event after event, and the footprint at any moment is just the current
event's live consumption on each thread. (When spilling is enabled the record
buffer keeps its reserved `LimitMemory` capacity between events to avoid
reallocating; in this case the scratch file's disk blocks are released and
reused each event.)

Staging requires output persistency. _remage_ resets its per-event staging
buffers in `ClearBeforeEvent`, which the framework only calls when an output
file is configured; without one the in-memory records — and any scratch files —
would accumulate across events without bound. Staging therefore aborts at run
start if any deferral is enabled while persistency is off; enable it by setting
an output file (`remage -o <file>` or
<project:../rmg-commands.md#rmgoutputfilename>). There should be no use case
where staging would be required without persistency.

The initial stage of Geant4 is stage 0 and this index is increased with every
stage transition. A stage transition is caused when the Geant4 `Urgent` stack is
empty, ignoring the custom stack. The custom stack will re-fill the `Urgent`
stack at every stage transition using the `ReinjectStagedTracks` function, until
the custom stack itself is empty. The simulation will transition to the next
event when all Geant4 stacks remain empty after this function has been called.

In _remage_, deferral is implemented only for tracks created in stage 0. Tracks
created in stage 1 and above that meet the defer-to-waiting conditions are
processed immediately (because at this point the condition, deciding if they
should be discarded, has already passed).

Suspension is implemented separately from track-initialization classification.
`RMGStagingScheme` implements a stepping action hook (`SteppingAction`) for
optional suspension of secondary electrons when they cross from above to below a
configured energy threshold. This is implemented by calling
`SetTrackStatus(fSuspend)` on the track, which halts processing and places the
track on the waiting stack. Suspended tracks will never be handled with custom
stacks.

### Caveats regarding electron staging

Electron staging is more physics-sensitive than optical-only staging because
electrons can carry significant energy in electromagnetic cascades.

If stage transition conditions depend on energy deposition in specific volumes
(for example `Germanium`), deferring electrons can move part of the energy
deposit in the conditioned volume to a later stage. This can make a
stage-transition condition fail in the initial stage and discard the waiting
stack. Electron staging can still be viable if a safety-distance condition is
applied so that only tracks far from Germanium detector surfaces are deferred.
To do this, use `VolumeSafety` and `AddVolumeName` to defer tracks only in
controlled regions. The best values depend on geometry and source type, but
`20 cm` is often a conservative starting point for far-away sources.

If stage transition conditions depend on isotope production (for example
`IsotopeFilter`), deferring high-energy shower components may suppress relevant
production channels in the initial stage. To mitigate this, use
`MaxEnergyThresholdForStacking` so only lower-energy tracks are deferred. The
threshold should be tuned for the isotope and reaction channels of interest.
This threshold applies to both staging and suspension.

As already mentioned above, staging affects filtering track output by process
(with <project:../rmg-commands.md#rmgoutputtrackaddprocessfilter>). Generally,
with staging electrons, the relationship between the parent/child track ids in
the output are broken.

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
crash the simulation. The custom staging backend (`RMGDeferring`, optionally
with `StorePath`) addresses the memory directly, but it does not completely
remove the per-track cost of generating the photon tracks in the first place.

To cut that per-track cost, electron staging can be beneficial: deferring the
secondary electrons keeps their sub-showers, and the optical photons those would
have produced, out of the initial stage. Due to the smaller number of electrons,
the positioning on the waiting stack is not a bottleneck, and one might get
significant performance improvements. However, considering the caveats described
above, it is important to validate the setup for the physics observables of
interest. This improvement is only relevant when the fraction of events skipping
the waiting stack is small (<1%). Examples of speedup can be found in the
validation section of the documentation.

**Therefore**, electron staging is recommended for cases with potentially many
optical photons, such as far-away radiogenic, and cosmogenic simulations. In
other cases, such as close-by sources, it may be preferable to stage only
optical photons.

## Commands

### Optical-photon staging

- <project:../rmg-commands.md#rmgstagingopticalphotonsdefertowaitingstage>
  enables deferral of optical photons to the waiting stack during stage 0.
- <project:../rmg-commands.md#rmgstagingopticalphotonsrmgdeferring> enables the
  compact, memory-efficient custom staging backend for optical photons instead
  of keeping full `G4Track`s on the waiting stack (see
  [Implementation details](#implementation-details)). Requires
  `DeferToWaitingStage`.
- <project:../rmg-commands.md#rmgstagingopticalphotonsstorepath> sets a
  directory in which the compact optical-photon records may spill to a scratch
  file, capping the recording footprint. If unset, the records are kept in
  memory. Requires `RMGDeferring`.
- <project:../rmg-commands.md#rmgstagingopticalphotonslimitmemory> sets the
  per-thread memory budget in MB (default `510`). It bounds the rebuilt
  `G4Track`s re-injected per stage, and - when `StorePath` is set - the
  threshold at which the record buffer spills to disk. Requires `RMGDeferring`.

### Electron staging

- <project:../rmg-commands.md#rmgstagingelectronsdefertowaitingstage> enables
  deferral of secondary electrons.
- <project:../rmg-commands.md#rmgstagingelectronsincludepositrons> additionally
  applies electron staging to secondary positrons. Disabled by default. When
  enabled, positrons are subject to the exact same conditions as electrons
  (energy thresholds, volume safety and volume names) and are also suspended on
  energy drop if `SuspendOnEnergyDrop` is enabled.
- <project:../rmg-commands.md#rmgstagingelectronsvolumesafety> sets a minimum
  distance to a Germanium detector surface condition.
- <project:../rmg-commands.md#rmgstagingelectronsmaxenergythresholdforstacking>
  limits deferred electrons to those with kinetic energy below the threshold.
- <project:../rmg-commands.md#rmgstagingelectronsminenergythresholdforstacking>
  limits deferred electrons to those with kinetic energy above the threshold,
  e.g. to skip low-energy electrons below the Cherenkov threshold.
- <project:../rmg-commands.md#rmgstagingelectronsaddvolumename> restricts
  deferral to named logical volumes.
- <project:../rmg-commands.md#rmgstagingelectronssuspendonenergydrop> enables
  stepping-time suspension when a track crosses from above to below the
  configured threshold.
- <project:../rmg-commands.md#rmgstagingelectronsrmgdeferring> enables the
  compact custom staging backend for electrons (and, with `IncludePositrons`,
  positrons) instead of keeping full `G4Track`s on the waiting stack (see
  [Implementation details](#implementation-details)). Requires
  `DeferToWaitingStage`.
- <project:../rmg-commands.md#rmgstagingelectronsstorepath> sets a directory in
  which the compact electron records may spill to a scratch file, capping the
  recording footprint. If unset, the records are kept in memory. Requires
  `RMGDeferring`.
- <project:../rmg-commands.md#rmgstagingelectronslimitmemory> sets the
  per-thread memory budget in MB (default `120`). It bounds the rebuilt
  `G4Track`s re-injected per stage, and - when `StorePath` is set - the
  threshold at which the record buffer spills to disk. Requires `RMGDeferring`.

### Suspension behavior

- Suspension is evaluated in stepping, not at end-of-track.
- The threshold is reused from `MaxEnergyThresholdForStacking` for the
  corresponding particle class.
- Suspension is applied only to secondary tracks.
- Suspension is always handled with the native Geant4 stacking.

### Stage transition conditions

You can configure conditions that clear waiting tracks at stage transition.
These are separate from the defer-to-waiting staging commands.

- **Germanium condition:**
  <project:../rmg-commands.md#rmgoutputgermaniumdiscardwaitingtracksunlessgermaniumedep>
  clears waiting tracks unless Germanium energy deposition occurred in the
  event.
- **IsotopeFilter condition:**
  <project:../rmg-commands.md#rmgoutputisotopefilterdiscardwaitingtracksunlessisotopeproduced>
  clears waiting tracks unless one of the configured isotopes was produced.
  Typical setup also requires:
  <project:../rmg-commands.md#rmgoutputactivateoutputscheme> with
  `IsotopeFilter` and at least one
  <project:../rmg-commands.md#rmgoutputisotopefilteraddisotope> command.

## Configuration checklist

1. Activate required optional output schemes (required: `Staging`, stage
   transition conditions: e.g., `IsotopeFilter`, or `Germanium`).
2. Specify which particles to defer to the waiting stack (e.g., optical photons,
   electrons) with the `Staging` commands.
3. Define the conditions for stage transition (e.g., energy deposition in
   Germanium, isotope production) with the relevant output scheme or filter
   commands.
4. (Optional) Enable suspension for electrons.
5. (Optional) For memory-heavy runs, enable the custom staging backend with
   `RMGDeferring`, and cap its footprint by setting `StorePath` (a directory on
   a real disk) and `LimitMemory`.
6. (Optional) Tune safety distances and energy thresholds based on source,
   geometry, and physics observables.

## Examples

The examples below are intended as a skeleton. Replace placeholders with your
actual geometry volume names, isotopes, and source definitions.

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
tracks after they cross below the configured energy threshold. Because a muon
shower produces enormous numbers of secondaries, the custom backend
(`RMGDeferring`) with a `StorePath` on a real disk and a per-thread
`LimitMemory` budget is used here to keep the staged optical-photon and electron
populations from exhausting memory; replace `/path/to/scratch` with a suitable
directory.

```geant4
/RMG/Output/ActivateOutputScheme Staging
/RMG/Output/ActivateOutputScheme IsotopeFilter

/run/initialize

/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Staging/OpticalPhotons/RMGDeferring true
/RMG/Staging/OpticalPhotons/StorePath /path/to/scratch
/RMG/Staging/OpticalPhotons/LimitMemory 512

/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/RMGDeferring true
/RMG/Staging/Electrons/StorePath /path/to/scratch
/RMG/Staging/Electrons/LimitMemory 128
/RMG/Staging/Electrons/IncludePositrons true
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 8.0 MeV
/RMG/Staging/Electrons/SuspendOnEnergyDrop true
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

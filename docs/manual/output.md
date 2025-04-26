(manual-output)=

# Output

:::{todo}

- one-table versus multi-table configurations
- vertex table
- track output scheme
- isotope, energy filtering
  :::

_remage_ supports all output formats supported by
[`G4AnalysisManager`](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Analysis/managers.html)
(HDF5, ROOT, CSV, XML), plus
[LH5](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5). The file
type to use is selected by the specified output file name (`.h5`, `.root`,
`.csv`, `.xml`, `.lh5`).

:::{note}
LH5, HDF5 and ROOT output formats require Geant4 to be explicitly compiled with
support for the HDF5 or ROOT libraries, respectively.
:::

The contents of the output files is determined by _output schemes_. An output
scheme does not only contain functionality for the actual output description,
but also might have parts of Geant4's stacking action functionality. Output
schemes, in general, are remage's way to implement pluggable event selection,
persistency and track stacking.

## Selection of output schemes

Adding a sensitive detector of any type will add the corresponding main output
scheme to the list of active output schemes.

Additional output schemes might be used for **filtering output**. Optional
output schemes can be enabled with the macro command
`/RMG/Manager/ActivateOutputScheme [name]`.

:::{note}
Adding output schemes with C++ code is possible using the `RMGUserInit` system
of remage (access it with
`auto user_init = RMGManager::Instance()->GetUserInit()`:

- `user_init->AddOutputScheme<T>(...);` adds and enables the output scheme
  `new T(...)` on each worker thread,
- `user_init->AddOptionalOutputScheme<T>("name", ...);` adds a name-tag to an
  output scheme, that will not be enabled right away, and
- `user_init->ActivateOptionalOutputScheme("name")` enables such a registered
  output scheme.

Output schemes are often coupled to
[sensitive detector types](project:./geometry.md#registering-sensitive-detectors).
At present, it is not possible to register detector types at runtime.
:::

## LH5 output

It is possible to directly write a LH5 file from remage, to facilitate reading
output ntuples as a
[LH5 Table](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/#Table).
To use this feature, simply specify an output file with a `.lh5` extension, and
remage will perform the file conversion automatically.

:::{note}
Additionally, the standalone tool `remage-to-lh5` is provided to convert a
default Geant4 HDF5 file to a LH5 file. With this, executing
`remage -o output.lh5 [...]` is roughly equivalent to the combination of
commands:

```console
$ remage -o output.hdf5 [...]
$ remage-to-lh5 output.hdf5
$ mv output.{hdf5,lh5}
```

:::

## Physical units

In LH5 output files, units are attached as attributes to the table columns, as
specified in the LH5 spec.

For any other output format (HDF5, ROOT, etc), remage is not able to attach
metadata to columns. The ntuple columns created by remage contain physical
units in their names, encoded as in the [legend-metadata
spec](https://legend-exp.github.io/legend-data-format-specs/dev/metadata/#Physical-units)
(i.e. adding `_in_<units>` at the end of the name), where the units are
expressed in the typical physical unit symbols. Unfortunately, column names
cannot contain forward slashes, so units lkike `m/s` are not possible to
represent directly. Instead, a backslash (`\`) is used to encode the division
symbol (for example: `velocity_in_m\s`).

## Germanium (HPGe) detectors

The Germanium output scheme handles the output from Germanium (HPGe) detectors, but would
also work for other solid state detectors (calorimeters).

Germanium detectors have sensitivity to the topology of event interactions via the pulse
shape and they also have a different response close to the detector electrodes. So
when simulating HPGe's it is advisable to save the information of the steps of
particles within the detector. Then "post-processing" software such as _reboost_
([docs](https://reboost.readthedocs.io/en/stable/)) can apply the detector response
model without repeating the computationally intensive simulation.

By default this output scheme writes out all steps in the registered sensitive
HPGe detectors. The following properties of each hit are recorded (by default):

- _time_: The global time of the hit,
- _particle_: the PDG code of the particle,
- _xloc_, _yloc_, _zloc_: the global position,
- _evtid_ : the index of the Geant4 event,
- _edep_: the deposited energy,
- _dist_to_surf_: the distance of the hit from the detector surface.

By default all floating point fields are saved with 64-bit (double) precision.
The precision of the energy and or position / distance fields can be reduced
to 32-bit with the macro commands:

`/RMG/Output/Germanium/StoreSinglePrecisionPosition true`

and,

`/RMG/Output/Germanium/StoreSinglePrecisionEnergy true`

It is possible to also store the track id and parent track id of each step with:

`/RMG/Output/Germanium/StoreTrackID true`

As mentioned earlier output schemes also provide a mechanism for filtering
events. One useful option is to only write out events in which energy
was deposited in a Germanium detector. This is used since the
other detector systems (LAr, water Cherenkov etc.) often act a "vetos's",
so we are not interested in the energy deposited or steps if an event in the
Germanium did not occur.
The macro commands:

`/RMG/Output/Germanium/AddDetectorForEdepThreshold {DET_UID}`
`/RMG/Output/Germanium/SetEdepCutLow {ELOW}`
`/RMG/Output/Germanium/SetEdepCutHigh {EHIGH}`

implement this functionality, for every event the total energy deposited is
computed. This is based on summing the energy deposited in each `{DET_UID}` added,
or across all registered sensitive Germanium detectors (if this macro command is not used).
The event is then discarded if the energy is less than or equal to `ELOW` or less than `EHIGH`.

:::{note}
This mechanism will remove the data from the event across all output schemes, not only the Germanium!
:::

Similarly, for simulations involving optical photons it is possible to discard all optical
photon tracks before simulating them if no energy was deposited in Germanium. This can be enabled
with the:

`/RMG/Output/Germanium/DiscardPhotonsIfNoGermaniumEdep`

macro command.

By default, the position saved for each step is the average of the pre and post-step point. This can
be controlled with the macro command:

`/RMG/Output/Germanium/StepPositionMode`

which can be set to `Average` (the default), `Both` (saves) also the pre and post steps, or `Pre`/`Post`.

:::{important}
For gammas the position saved is always that of the post-step, since all gamma interactions are discrete.
:::

Typically only steps where some energy was deposited are written out to disk, to control
this behaviour there is a macro command:

`/RMG/Output/Germanium/DiscardZeroEnergyHits`

Finally, it is possible to "pre-cluster" the steps, this is used to reduce the
amount of data written out to disk by combining steps very close together.
Since the surface region of a HPGe detector has different properties to the bulk
this clustering can be performed differently for surface and bulk hits
(see [data-reduction](#data-reduction-methods) for more details).

## Scintillator detectors

This output scheme is used to record the steps in scintillation detectors (typically LAr),
this is a calometric approach recording the energy deposited and steps. While the Optical
output scheme is instead used for recording the detected optical photons.
Most functionality is similar to the Germanium output scheme with a few exceptions.

- Unlike for Germanium detectors the distance to the detector surface is not calculated,
- The stacking possibility for optical tracks is not implemented,
- The velocity of the particles can be saved using the

`/RMG/Output/Scintillator/StoreParticleVelocities`

macro command.

## Data reduction methods

Often Geant4 takes steps much shorter than those that are meaningful in a HPGe or a
scintillation detector. For example the typical dimension of charge clouds
produced by interactions in Germanium are 1-2 mm, so we are not sensitive to tracking at um levels.
To reduce the file size while retaining the useful information for computing observables of interest
we have implemented some "pre-clustering" routines. These routines combine together steps that are very
very close together.

:::{note}
The aim of this (pre)-clustering is only to make a minimal reduction of information which cannot be useful!
Further, more aggressive clustering may be needed for some applications.
:::

In order to have an efficient algorithm for pre-clustering we take use a "within-track" approach,
this clusters only steps in the same `G4Track`, with some exceptions for very low energy tracks.
In this way we only have to iterate through the steps in each event once. This also means the rows in our output are still interpretable
with steps in the detector (just with a larger step length). The clustering is handled by the
function:

```C++
  RMGDetectorHitsCollection* pre_cluster_hits(const RMGDetectorHitsCollection* hits,
      ClusterPars cluster_pars, bool has_distance_to_surface, bool has_velocity);
```

This takes in the pointer to the original `RMGDetectorHitsCollection` returning a pointer
to a new collection of clustered hits.

:::{note}
This design makes it easy to include additional clustering algorithms, a similar function just needs to be written.
:::
Pre-clustering can be enabled with the macro command:

`/RMG/Output/Germanium/PreClusterOutputs True`

and similar for the `Scintillator` output scheme.

_remage_ first organises the hits by track id. Some processes in _Geant4_ produce
a large number of secondary tracks due to atomic de-excitation, these tracks typically
have a very low energy and range (however they are still produced since production cuts
are not applied for most gamma interactions). Thus they are not expected to impact observables of interest.
In many cases, after pre-clustering of high energy electrons, these tracks could form the majority
of the output. We implemented the possibility to merge these tracks prior to pre-clustering
which can be enabled with the macro command:

`/RMG/Output/Germanium/CombineLowEnergyElectronTracks True`

or similar for the Scintillator output.

:::{warning}
This means in some cases there are steps in the output that are the combination of steps in different
Geant4 tracks.
:::

This command will select electron tracks with energy lower than a threshold, which is by default 10 keV,
but can be changed with the macro command:

`/RMG/Output/Germanium/SetElectronTrackEnergyThreshold {ENERGY}`

and similar for the Scintillator output. For each track, we search for tracks which have a first pre-step
point within the cluster radius of the first pre-step point of the low energy track. The low energy track
is then merged with the neighbour track with the highest energy. In addition, Geant4 will sometimes
associated some deposited energy with gamma tracks (due to atomic binding energy), optionally the
user can request instead redistributing this energy to the secondary electron tracks with the command:

`/RMG/Output/Germanium/RedistributeGammaEnergy True`

this then means the gamma tracks would not have energy deposits and do not need to be written out in the
output file (unless this is explicitly requested).

After these two pre-processing steps the pre-clustering proceeds by looping through the steps in each
track. For each step the distance to the first step in the current cluster is calculated, if this distance
is less than the user defined distance, and the time difference is less than the time threshold,
the step is added to the current cluster.

The distance / time thresholds used for pre-clustering can be set with:

`/RMG/Output/Germanium/SetPreClusterDistance`

`/RMG/Output/Germanium/SetPreClusterTimeThreshold`

and similar for the Scintillator output scheme.

Germanium detectors, where the surface region has substantially different properties to the bulk, we
give the possibility to cluster with a different threshold for the surface region of the detector.
This is by default the region within 2 mm of the detector surface but can be changed with the command:

`/RMG/Output/Germanium/SetSurfaceThickness`

then a threshold can be set specifically for this region with:

`/RMG/Output/Germanium/SetPreClusterDistanceSurface`

this will apply this threshold for any step where the distance to surface is less than the surface
thickness. With this option a new cluster will also be formed if a step moves from the surface to bulk region
of the Germanium (or visa versa). With these options we provide a sophisticated mechanism for handling
the surface of Germanium detectors.

For each cluster, we then compute an "effective" step, where the time, pre-step position, distance to surface,
velocity is taken from the first step. The post-step position, distance are evalauted from the last step while the
energy deposit is summed over all steps. The average of the pre-step position and post-step position is computed.
All other fields are constant within a track and are taken from the first step.

:::{note}
In this way the output still represents a step, just with a longer effective step length.
:::

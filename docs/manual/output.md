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
output schemes can be enabled with the
<project:../rmg-commands.md#rmgoutputactivateoutputscheme> macro command:

`/RMG/Output/ActivateOutputScheme [name]`.

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

## Output file types

The selection of the output file type depends on the file extension of the specified
output file. Possible output file types include `lh5`, `hdf5`, or `root`—or any
other file format that `G4AnlasisManager` can write; but these are not tested
regularly.

:::{note}
remage will not produce an output file, if no output file name is provided by the
user. Specify `-o none` to acknowledge the warning that is emitted when output
schemes are registered, but no file will be created.
:::

## LH5 output

It is possible to directly write a LH5 file from _remage_, to facilitate reading
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
cannot contain forward slashes, so units like `m/s` cannot be represent
directly. Instead, a backslash (`\`) is used to encode the division symbol (for
example: `velocity_in_m\s`).

## Germanium (HPGe) detectors

The _Germanium_ output scheme handles the output from germanium (HPGe)
detectors, but would also work for other solid state detectors (calorimeters).

HPGes have sensitivity to the topology of event interactions via the pulse
shape and they also have a different response close to the detector electrodes.
So when simulating HPGe's it is advisable to save the information of the steps
of particles within the detector. Then "post-processing" software such as
[_reboost_](https://reboost.readthedocs.io/en/stable/) can apply the detector
response model without repeating the computationally intensive simulation.

By default this output scheme writes out all steps in the registered sensitive
HPGe detectors. The following properties of each hit are recorded (by default):

- `time`: The global time of the hit,
- `particle`: the PDG code of the particle,
- `xloc`, `yloc`, `zloc`: the global position,
- `evtid_`: the index of the Geant4 event,
- `edep`: the deposited energy,
- `dist_to_surf`: the distance of the hit from the detector surface.

By default all floating point fields are saved with 64-bit (double) precision.
The precision of the energy and or position / distance fields can be reduced
to 32-bit with the macro commands
<project:../rmg-commands.md#rmgoutputgermaniumstoresingleprecisionposition> and
<project:../rmg-commands.md#rmgoutputgermaniumstoresingleprecisionenergy>.

It is possible to also store the track id (see
[link](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/tracking.html)
for details) and parent track id of each step with
<project:../rmg-commands.md#rmgoutputgermaniumstoretrackid>.

As mentioned earlier output schemes also provide a mechanism for filtering
events. One useful option is to only write out events in which energy
was deposited in a germanium detector. This is used since the
other detector systems (liquid argon, water Cherenkov etc.) often act a
"vetos's", so we are not interested in the energy deposited or steps if an
event in the germanium did not occur. The macro commands
<project:../rmg-commands.md#rmgoutputgermaniumadddetectorforedepthreshold>,
<project:../rmg-commands.md#rmgoutputgermaniumedepcutlow> and
<project:../rmg-commands.md#rmgoutputgermaniumedepcuthigh>:

```remage
/RMG/Output/Germanium/AddDetectorForEdepThreshold {DET_UID}
/RMG/Output/Germanium/EdepCutLow {ELOW}
/RMG/Output/Germanium/EdepCutHigh {EHIGH}
```

implement this functionality, for every event the total energy deposited is
computed. This is based on summing the energy deposited in each `{DET_UID}`
added, or across all registered sensitive _Germanium_ detectors (if this macro
command is not used). The event is then discarded if the energy is less than
or equal to `ELOW` or less than `EHIGH`.

:::{note}
This mechanism will remove the data from the event across all output schemes,
not only the _Germanium_!
:::

Similarly, for simulations involving optical photons it is possible to discard
all optical photon tracks before simulating them if no energy was deposited in
germanium. This can be enabled with
<project:../rmg-commands.md#rmgoutputgermaniumdiscardphotonsifnogermaniumedep>.

By default, the position saved for each step is the average of the pre and
post-step point. This can be controlled with
<project:../rmg-commands.md#rmgoutputgermaniumsteppositionmode>, which can be
set to `Average` (the default), `Both` (saves) also the pre and post steps, or
`Pre`/`Post`.

:::{important}
For gammas the position saved is always that of the post-step, since all gamma
interactions are discrete.
:::

Typically only steps where some energy was deposited are written out to disk,
to control this behaviour there is
<project:../rmg-commands.md#rmgoutputgermaniumdiscardzeroenergyhits>.

Finally, it is possible to "pre-cluster" the steps, this is used to reduce the
amount of data written out to disk by combining steps very close together.
Since the surface region of a HPGe detector has different properties to the bulk
this clustering can be performed differently for surface and bulk hits
(see [data-reduction](#data-reduction-methods) for more details).

## Scintillator detectors

This output scheme is used to record the steps in scintillation detectors
(typically liquid argon), this is a calometric approach recording the energy
deposited and steps. While the _Optical_ output scheme is instead used for
recording the detected optical photons. Most functionality is similar to the
_Germanium_ output scheme with a few exceptions.

- Unlike for germanium detectors the distance to the detector surface is not
  calculated,
- The stacking possibility for optical tracks is not implemented,
- The velocity of the particles can be saved using the
  <project:../rmg-commands.md#rmgoutputscintillatorstoreparticlevelocities>
  command.

## Data reduction methods

Often Geant4 takes steps much shorter than those that are meaningful in a HPGe
or a scintillation detector. For example the typical dimension of charge clouds
produced by interactions in germanium are 1-2 mm, so we are not sensitive to
tracking at the micrometer level. To reduce the file size while retaining the
useful information for computing observables of interest we have implemented
some "pre-clustering" routines. These routines combine together steps that are
very close together.

:::{note}
The aim of this (pre)-clustering is only to make a minimal reduction of
information which cannot be useful! Further, more aggressive clustering may be
needed for some applications.
:::

In order to have an efficient algorithm for pre-clustering we take use a
"within-track" approach, this clusters only steps in the same
`G4Track`, with some exceptions for very low energy tracks. In this way we only
have to iterate through the steps in each event once. This also means the rows
in our output are still interpretable with steps in the detector (just with a
larger step length). The clustering is handled by the
function {cpp:func}`RMGOutputTools::pre_cluster_hits`. This takes in the
pointer to the original {cpp:type}`RMGDetectorHitsCollection` returning a
pointer to a new collection of clustered hits.

:::{note}

- The function returns a shared pointer to the hit collection, for some
  applications it may be necessary to extract also an unmanaged pointer, for
  example to make this collection look identical to that obtained directly from
  Geant4.
- This design makes it easy to include additional clustering algorithms, a
  similar function just needs to be written.
  :::

Pre-clustering can be enabled with
<project:../rmg-commands.md#rmgoutputgermaniumclusterpreclusteroutputs> and
similarly for the _Scintillator_ output scheme:
<project:../rmg-commands.md#rmgoutputscintillatorclusterpreclusteroutputs>.

We first organise the hits by track id (the index of the `G4Track` within the
event). Some processes in Geant4 produce a large number of secondary tracks due
to atomic de-excitation, these tracks typically have a very low energy and
range (however they are still produced since production cuts are not applied
for most gamma interactions). Thus they are not expected to impact observables
of interest. In many cases, after pre-clustering of high energy electrons,
these tracks could form the majority of the output.

We implemented the possibility to merge these tracks prior to pre-clustering
which can be enabled with
<project:../rmg-commands.md#rmgoutputgermaniumclustercombinelowenergyelectrontracks>
and similarly for the _Scintillator_ output scheme:
<project:../rmg-commands.md#rmgoutputscintillatorclustercombinelowenergyelectrontracks>.

:::{warning}
This means in some cases there are steps in the output that are the combination
of steps in different Geant4 tracks.
:::

This command will select electron tracks with energy lower than a threshold,
which is by default 10 keV, but can be changed with
<project:../rmg-commands.md#rmgoutputgermaniumclusterelectrontrackenergythreshold>.

and similar for the _Scintillator_ output scheme:
<project:../rmg-commands.md#rmgoutputscintillatorclusterelectrontrackenergythreshold>.
For each track, we search for tracks which have a first pre-step point within
the cluster radius of the first pre-step point of the low energy track. The low
energy track is then merged with the neighbour track with the highest energy.
In addition, Geant4 will sometimes associated some deposited energy with gamma
tracks (due to atomic binding energy), optionally the user can request instead
redistributing this energy to the secondary electron tracks with
<project:../rmg-commands.md#rmgoutputgermaniumclusterredistributegammaenergy>.

this then means the gamma tracks would not have energy deposits and do not need
to be written out in the output file (unless this is explicitly requested). Or
similarly for the _Scintillator_ output scheme:
<project:../rmg-commands.md#rmgoutputscintillatorclusterredistributegammaenergy>.

After these two pre-processing steps the pre-clustering proceeds by looping
through the steps in each track. For each step the distance to the first step
in the current cluster is calculated, if this distance is less than the user
defined distance, and the time difference is less than the time threshold, the
step is added to the current cluster.

The distance/time thresholds used for pre-clustering can be set with the
commands
<project:../rmg-commands.md#rmgoutputgermaniumclusterpreclusterdistance> and
<project:../rmg-commands.md#rmgoutputgermaniumclusterpreclustertimethreshold>,
and similar for the _Scintillator_ output scheme:
<project:../rmg-commands.md#rmgoutputscintillatorclusterpreclusterdistance> and
<project:../rmg-commands.md#rmgoutputscintillatorclusterpreclustertimethreshold>

For _Germanium_ detectors, where the surface region has substantially different
properties to the bulk, we give the possibility to cluster with a different
threshold for the surface region of the detector. This is by default the
region within 2 mm of the detector surface but can be changed with
<project:../rmg-commands.md#rmgoutputgermaniumclustersurfacethickness>.
Then, a threshold can be set specifically for this region with
<project:../rmg-commands.md#rmgoutputgermaniumclusterpreclusterdistancesurface>.
This will apply this threshold for any step where the distance to surface is
less than the surface thickness. With this option a new cluster will also be
formed if a step moves from the surface to bulk region of the germanium (or
vice-versa).

These options provide a sophisticated mechanism for handling the surface of
HPGe detectors!

For each cluster, we then compute an "effective" step:

- the time, pre-step position, distance to surface, velocity is taken from the first step.
- the post-step position, distance are evalauted from the last step
- while the energy deposit is summed over all steps.
- the average of the pre-step position and post-step position is computed.

All other fields are constant within a track and are taken from the first step.

:::{note}
In this way the output still represents a step, just with a longer effective step length.
:::

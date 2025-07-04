(manual-output)=

# Output

:::{todo}

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
schemes, in general, are _remage_'s way to implement pluggable event selection,
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
of _remage_ (access it with
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

The selection of the output file type depends on the file extension of the
specified output file. Possible output file types include `lh5`, `hdf5`, or
`root`—or any other file format that `G4AnalysisManager` can write; but these
are not tested regularly. In general, we recommend choosing the
[LH5](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/) format
(more below).

:::{note}

_remage_ will not produce an output file, if no output file name is provided by
the user. Specify `-o none` to acknowledge the warning that is emitted when
output schemes are registered, but no file will be created.

:::

In case a multithreaded simulation is requested with the `-t` or `--threads`
option (see {ref}`manual-running`), the output file names will be appended with
the thread number. _remage_ will produce one output file per thread appending
`_t$id`, where `$id` is the thread number, before the file extension.

For example running _remage_ with:

```
remage -o OUTPUT.lh5 -t 8
```

will result in output files `OUTPUT_t0.lh5,..., OUTPUT_t7.lh5`.

Geant4 automatically merges these files into a single one at the end of a run
for all supported formats, except for HDF5. For the LH5 output format, _remage_
can merge the output files before saving to disk. This feature can be enabled
with the `--merge-output-files` (or `-m`) command line option.

:::{warning}

Merging involves some additional I/O operations so for some simulations may
increase run time! _remage_ will report the amount of time spent merging the
files.

:::

## Physical units

In LH5 output files, units are attached as attributes to the table columns, as
specified in the LH5 spec.

For any other output format (HDF5, ROOT, etc), remage is not able to attach
metadata to columns. The ntuple columns created by remage contain physical units
in their names, encoded as in the
[legend-metadata spec](https://legend-exp.github.io/legend-data-format-specs/dev/metadata/#Physical-units)
(i.e. adding `_in_<units>` at the end of the name), where the units are
expressed in the typical physical unit symbols. Unfortunately, column names
cannot contain forward slashes, so units like `m/s` cannot be represent
directly. Instead, a backslash (`\`) is used to encode the division symbol (for
example: `velocity_in_m\s`).

## Germanium (HPGe) detectors

The _Germanium_ output scheme handles the output from germanium (HPGe)
detectors, but would also work for other solid state detectors (calorimeters).

HPGes have sensitivity to the topology of event interactions via the pulse shape
and they also have a different response close to the detector electrodes. So
when simulating HPGe's it is advisable to save the information of the steps of
particles within the detector. Then "post-processing" software such as
[_reboost_](https://reboost.readthedocs.io/en/stable/) can apply the detector
response model without repeating the computationally intensive simulation.

By default this output scheme writes out all steps in the registered sensitive
HPGe detectors. The following properties of each hit are recorded (by default):

- `time`: The global time of the hit,
- `particle`: the PDG code of the particle,
- `xloc`, `yloc`, `zloc`: the global position,
- `evtid`: the index of the Geant4 event,
- `edep`: the deposited energy,
- `dist_to_surf`: the distance of the hit from the detector surface.

By default all floating point fields are saved with 64-bit (double) precision.
The precision of the energy and or position / distance fields can be reduced to
32-bit with the macro commands
<project:../rmg-commands.md#rmgoutputgermaniumstoresingleprecisionposition> and
<project:../rmg-commands.md#rmgoutputgermaniumstoresingleprecisionenergy>.

It is possible to also store the track id (see
[link](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/tracking.html)
for details) and parent track id of each step with
<project:../rmg-commands.md#rmgoutputgermaniumstoretrackid>.

As mentioned earlier output schemes also provide a mechanism for filtering
events. One useful option is to only write out events in which energy was
deposited in a germanium detector. This is used since the other detector systems
(liquid argon, water Cherenkov etc.) often act a "vetos's", so we are not
interested in the energy deposited or steps if an event in the germanium did not
occur. The macro commands
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
command is not used). The event is then discarded if the energy is less than or
equal to `ELOW` or less than `EHIGH`.

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

Typically only steps where some energy was deposited are written out to disk, to
control this behaviour there is
<project:../rmg-commands.md#rmgoutputgermaniumdiscardzeroenergyhits>.

Finally, it is possible to "pre-cluster" the steps, this is used to reduce the
amount of data written out to disk by combining steps very close together. Since
the surface region of a HPGe detector has different properties to the bulk this
clustering can be performed differently for surface and bulk hits (see
[data-reduction](#data-reduction-methods) for more details).

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

## Single- versus multi-detector table layout

_remage_ will store sensitive volume hits in separate output tables by default,
one per detector. While this layout is useful if analyzing data from each
detector independently, sometimes having all hits stored in the same output
table can be more beneficial. The multi-table layout can be disabled by setting
<project:../rmg-commands.md#rmgoutputntupleperdetector> to false. In this
scenario, _remage_ will create a table for each detector category (`Germanium`,
`Scintillator`, ...).

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
"within-track" approach, this clusters only steps in the same `G4Track`, with
some exceptions for very low energy tracks. In this way we only have to iterate
through the steps in each event once. This also means the rows in our output are
still interpretable with steps in the detector (just with a larger step length).
The clustering is handled by the function
{cpp:func}`RMGOutputTools::pre_cluster_hits`. This takes in the pointer to the
original {cpp:type}`RMGDetectorHitsCollection` returning a pointer to a new
collection of clustered hits.

:::{note}

- The function returns a shared pointer to the hit collection, for some
  applications it may be necessary to extract also an unmanaged pointer, for
  example to make this collection look identical to that obtained directly from
  Geant4.
- This design makes it easy to include additional clustering algorithms, a
  similar function just needs to be written.

:::

Pre-clustering is enabled by default for the Scintillator and Germanium output
schemes, it can be disabled with the command
<project:../rmg-commands.md#rmgoutputgermaniumclusterpreclusteroutputs> and
similarly for the _Scintillator_ output scheme:
<project:../rmg-commands.md#rmgoutputscintillatorclusterpreclusteroutputs>.

This clustering works by by first organise the hits by track id (the index of
the `G4Track` within the event). Some processes in Geant4 produce a large number
of secondary tracks due to atomic de-excitation, these tracks typically have a
very low energy and range (however they are still produced since production cuts
are not applied for most gamma interactions). Thus they are not expected to
impact observables of interest. In many cases, after pre-clustering of high
energy electrons, these tracks could form the majority of the output.

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
energy track is then merged with the neighbour track with the highest energy. In
addition, Geant4 will sometimes associated some deposited energy with gamma
tracks (due to atomic binding energy), optionally the user can request instead
redistributing this energy to the secondary electron tracks with
<project:../rmg-commands.md#rmgoutputgermaniumclusterredistributegammaenergy>.

this then means the gamma tracks would not have energy deposits and do not need
to be written out in the output file (unless this is explicitly requested). Or
similarly for the _Scintillator_ output scheme:
<project:../rmg-commands.md#rmgoutputscintillatorclusterredistributegammaenergy>.

After these two pre-processing steps the pre-clustering proceeds by looping
through the steps in each track. For each step the distance to the first step in
the current cluster is calculated, if this distance is less than the user
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
threshold for the surface region of the detector. This is by default the region
within 2 mm of the detector surface but can be changed with
<project:../rmg-commands.md#rmgoutputgermaniumclustersurfacethickness>. Then, a
threshold can be set specifically for this region with
<project:../rmg-commands.md#rmgoutputgermaniumclusterpreclusterdistancesurface>.
This will apply this threshold for any step where the distance to surface is
less than the surface thickness. With this option a new cluster will also be
formed if a step moves from the surface to bulk region of the germanium (or
vice-versa).

:::{note}

By default pre-clustering is performed for both the _Germanium_ and
_Scintillator_ output schemes with 50 $\mu$m distance for Germanium. By default
clustering is not applied to the surface for _Germanium_ (within the surface
thickness set by default as 2 mm). For the _Scintillator_ output scheme we use
500 $\mu$ m cluster distance by default. For both outputs a 10$\mu$ m time
threshold is used by default.

:::

These options provide a sophisticated mechanism for handling the surface of HPGe
detectors!

For each cluster, we then compute an "effective" step:

- the time, pre-step position, distance to surface, velocity is taken from the
  first step.
- the post-step position, distance are evalauted from the last step
- while the energy deposit is summed over all steps.
- the average of the pre-step position and post-step position is computed.

All other fields are constant within a track and are taken from the first step.

:::{note}

In this way the output still represents a step, just with a longer effective
step length.

:::

## LH5 output

[LH5 (LEGEND-HDF5)](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/)
is a simple, open-source HDF5-based data format specification initially
developed by the LEGEND collaboration. Good support for reading and writing LH5
files is available in Python (through
[_legend-pydataobj_](https://legend-pydataobj.readthedocs.io)) and in Julia
(through [_LegendHDF5IO.jl_](...)). Alternatively, since the data is stored in
HDF5, it can be read by any other HDF5 I/O tool. Given the advantages of the LH5
format over the others, _remage_ adopts it as primary output format and
recommends it to all users.

It is possible to directly write a LH5 file from _remage_, to facilitate reading
output ntuples as a
[LH5 Table](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/#Table).
To use this feature, simply specify an output file with a `.lh5` extension, and
_remage_ will perform the file conversion automatically.

If the LH5 output is selected, _remage_ performs some post-processing steps at
the end of a simulation run such as re-organizing data into more meaningful
structures or adding useful information.

:::

### Reshaping output tables

For the LH5 format and _Germanium_ or _Scintillator_ outputs we implemented a
"reshaping" of the output tables. This groups together rows in the same output
table that have the same simulated Geant4 evtid and also with times with the
user defined time window (more later). In this way the resulting rows of the
output table represent physical interactions in a sensitive volume occurring in
a window compatible with the time resolution of the detector. In the following,
we will often refer to these as "hits".

The means the columns of the output table are converted from
[LH5 Array's](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/#Array)
objects to
[LH5 VectorOfVectors's](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/#Vector-of-vectors).
However, this grouping is lossless.

Without reshaping, the output table is flat: each column (`evtid`, `edep`, ...)
is a one-dimensional array:

```
[{evtid: 0, particle: 11, edep: 20.1, time: 0.158, xloc: -0.0222, ...},
 {evtid: 0, particle: 11, edep: 50.1, time: 0.251, xloc: -0.0178, ...},
 {evtid: 0, particle: 11, edep: 74.9, time: 0.522, xloc: -0.0767, ...},
 {evtid: 2, particle: 11, edep: 0.431, time: 0.344, xloc: -0.0335, ...},
 {evtid: 2, particle: 11, edep: 109, time: 0.423, xloc: -0.0542, ...},
 {evtid: 3, particle: 11, edep: 70.2, time: 0.0545, xloc: -0.0128, ...},
 ...,
 {evtid: 44, particle: 11, edep: 88.4, time: 0.484, xloc: -0.0313, ...},
 {evtid: 49, particle: 11, edep: 33.1, time: 0.848, xloc: -0.126, ...},
 {evtid: 49, particle: 11, edep: 115, time: 0.85, xloc: -0.125, yloc: ..., ...}]
```

With reshaping, columns acquire one additional dimension:

```
[{edep: [20.1, 50.1, ..., 74.9], evtid: [0, ..., 0], particle: ..., ...},
 {edep: [0.431, 109], evtid: [2, 2], particle: [11, 11], ...},
 {edep: [70.2], evtid: [3], particle: [11], time: [...], ...},
 ...,
 {edep: [88.4], evtid: [44], particle: [...], ...},
 {edep: [33.1, 115], evtid: [49, 49], particle: ..., ...}]
```

This behavior is enabled by default for `.lh5` file outputs, it can be
suppressed with the `--flat-output` flag to the _remage_ executable. The time
window used to group together rows can be set with the `--time-window-in-us`
flag, the units are $\mu$s and by default a window of 10$\mu$s is used.

:::{warning}

Reshaping involves some additional I/O operations so for some simulations may
increase run time! _remage_ will report the amount of time spent reshaping the
files.

:::

It is possible to supply both the `-m` and `-r` flags to simultaneously merge
and reshape the output files.

### Time-coincidence map

In the presence of multiple sensitive detectors written out as separate output
tables, reconstructing event information can be a tedious operation when
analyzing the simulation output. To simplify the task, _remage_ computes a
so-called "time-coincidence map" (TCM) table at the end of a simulation run with
{func}`pygama.evt.tcm.build_tcm` and stores it in the output file as `/tcm`.

Every row of the TCM corresponds to a simulated event, defined by the same time
window used for reshaping the output tables. For each event, the `table_key`
column specifies the list of detectors that had hits, while the `row_in_table`
column specifies which rows (through the row index) need to be read from the
respective output tables. The detectors are labeled in `row_in_table` by their
index in the `tables` HDF5 attribute.

More information on how to use the TCM is provided in {ref}`manual-analysis`,
while more documentation about how the TCM is generated is available at
{func}`pygama.evt.tcm.generate_tcm_cols`.

## The vertex table

_remage_ stores data about the simulated event vertex in a table named `vtx`. In
the LH5 output, it can be found in the HDF5 root group (`/vtx`). Each row in the
table corresponds to a vertex. The columns are:

- `evtid`: Geant4 event identifier
- `time`: time relative to the start of the event (zero, most of the times)
- `n_part`: number of generated particles
- `xloc`, `yloc`, `zloc`: global coordinates of the vertex position.

The table always lists data from all vertices, i.e. the length of the table is
always equal to the number of simulated events.

The vertex table is useful to reconstruct the event vertex of hits recorded in
sensitive detectors by matching the information stored in the `evtid` column.

(manual-output)=

# Output

_remage_ mainly supports the
[LH5](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5) output
format. This is the only format we can provide the convenient output reshaping
for.

Additionally, _remage_ supports—with limited functionality—all formats supported
by
[`G4AnalysisManager`](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Analysis/managers.html)
(HDF5, ROOT, CSV, XML). The file type to use is selected by the specified output
file name (`.hdf5`, `.root`, `.csv`, `.xml`, `.lh5`).

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

Adding a sensitive detector of any type (see
{ref}`manual-geometry-register-sens-det`) will add the corresponding main output
scheme to the list of active output schemes.

Additional output schemes might be used for **filtering output**. Optional
output schemes can be enabled with the
<project:../rmg-commands.md#rmgoutputactivateoutputscheme> macro command:

```geant4
/RMG/Output/ActivateOutputScheme {NAME}
```

where `{NAME}` is the name of the output scheme.

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

A similar naming scheme applies when using the multi-process based parallelism
(requested by `-P` or `--procs`), which will append a suffix of `_p$id`.

Geant4 automatically merges these files into a single one at the end of a run
for all supported formats, except for HDF5. For the LH5 output format, _remage_
can merge the output files before saving to disk. This feature can be enabled
with the `--merge-output-files` (or `-m`) command line option.

:::{warning}

Merging involves some additional I/O operations so for some simulations may
increase run time! _remage_ will report the amount of time spent merging the
files. Merging can take a lot of time as these operations are run on a single
core at the moment.

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

## Built-in output schemes

_remage_ currently implements schemes to read out `Germanium`, `Scintillator`
and `Optical` detector types.

### `Germanium`: Germanium (HPGe) detectors

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

```geant4
/RMG/Output/Germanium/AddDetectorForEdepThreshold {UID}
/RMG/Output/Germanium/EdepCutLow {ELOW}
/RMG/Output/Germanium/EdepCutHigh {EHIGH}
```

implement this functionality, for every event the total energy deposited is
computed. This is based on summing the energy deposited in each `{UID}` added,
or across all registered sensitive _Germanium_ detectors (if this macro command
is not used). The event is then discarded if the energy is less than or equal to
`ELOW` or more than `EHIGH`.

:::{note}

This mechanism will remove the data from the event across all output schemes,
not only the _Germanium_! However, `OutputSchemes` with their `StoreAlways()`
function returning `true`, like the `RMGTrackOutputScheme` or the
`RMGVertexOutputScheme` will always store their output.

:::

Similar commands are available for the scintillator output scheme.

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

### `Scintillator`: calorimeters

This output scheme records stepping data in scintillating materials (e.g. liquid
argon), following a calometric approach. This scheme is useful for applications
where an optical detector response is applied on the simulated particle
interactions. Detection of optical photons is handled by the _Optical_ output
scheme (see later). Most functionality is similar to the _Germanium_ output
scheme with a few exceptions:

- Unlike for germanium detectors the distance to the detector surface is not
  calculated,
- Stacking of optical tracks is not implemented,
- The velocity of the particles can be saved using the
  <project:../rmg-commands.md#rmgoutputscintillatorstoreparticlevelocities>
  command.

### `Optical`: optical photon detectors

This output scheme records photon hits data in detectors registered as optical
detectors. This scheme is useful for applications where an optical detector
response should be directly recorded and not produced in post-processing. It
records the time stamp and the wavelength of the detected photons.

:::{note}

Unlike the other detector types that work without geometry changes, the optical
detectors must fulfill other constraints in the user-defined geometry to be
useful:

- They need to have a optical surface set, with `REFLECTIVITY` < 1 and
  `EFFICIENCY` > 0
- The geometry must follow best practices for optixal geometry development. The
  most basic requirement is that the detector volumes must be reachable by
  optical paths (i.e., _all_ materials involved in the photon propagation have
  refractive indices set).

:::

## Single- versus multi-detector table layout

_remage_ will store sensitive volume hits in separate output tables by default,
one per detector. While this layout is useful if analyzing data from each
detector independently, sometimes having all hits stored in the same output
table can be more beneficial. The multi-table layout can be disabled by setting
<project:../rmg-commands.md#rmgoutputntupleperdetector> to false. In this
scenario, _remage_ will organize hits by detector type in separate tables (a
table named `germanium` for the `Germanium` detectors, `scintillator` for
`Scintillator`s, etc.).

(manual-output-table-naming)=

## Table naming

By default, _remage_ will name output tables by their internal unique identifier
(UID), prefixed with `det` (i.e. the table for a detector with UID 39 will be
saved as `det039`). These tables are located in a directory named `stp` (as for
"stepping data"). For example, an output LH5 file will look like:

```
/
└── stp · struct{det1104005,det1105600,...}
    ├── det1104005 · table{...}
    │   └── ...
    ├── det1105600 · table{...}
    │   └── ...
    └── ...
```

:::{note}

The directory name can be customized with the
<project:../rmg-commands.md#rmgoutputntupledirectory> command.

:::

Non-stepping-data (auxiliary) tablers are stored in the same directory, due to
limitations of the Geant4 analysis manager, except for the LH5 output (see
later).

_remage_ offers the possibility to name stepping data tables after their Geant4
logical volume name, instead of the UID. This feature can be enabled with the
<project:../rmg-commands.md#rmgoutputntupleusevolumename> command. In this case,
for example, an output LH5 file will look like:

```
/
└── stp · struct{B00000C,B00000D,...}
    ├── B00000C · table{...}
    │   └── ...
    ├── B00000D · table{...}
    │   └── ...
    └── ...
```

:::{note}

If the LH5 output format is selected, detector tables can be still accessed by
UID through the symbolic stored in the `/stp/__by_uid__` group.

```
/
└── stp · struct{B00000C,B00000D,...}
    ├── __by_uid__ · struct{det1104005,det1105600,...}
    │   ├── det1104005 -> /stp/B00000C
    │   ├── det1105600 -> /stp/B00000D
    │   └── ...
    ├── B00000C · table{...}
    │   └── ...
    ├── B00000D · table{...}
    │   └── ...
    └── ...
```

:::

## Data reduction methods

### Step (pre-)clustering

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

This clustering works by first organizing the hits by track id (the index of the
`G4Track` within the event). Some processes in Geant4 produce a large number of
secondary tracks due to atomic de-excitation, these tracks typically have a very
low energy and range (however they are still produced since production cuts are
not applied for most gamma interactions). Thus they are not expected to impact
observables of interest. In many cases, after pre-clustering of high energy
electrons, these tracks could form the majority of the output.

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

### Output filtering

Another strategy for output file size reduction is to filter out unwanted events
before even writing them to disk. In _remage_, these filters can be registered
as optional "output" schemes (do not get confused by the name, they will not
produce any additional output).

#### Isotope filter

The isotope filter is rather simple and can be enabled and used like this:

```geant4
/RMG/Output/ActivateOutputScheme IsotopeFilter
/RMG/Output/IsotopeFilter/AddIsotope {A} {Z}
```

Adds an isotope to the list. Only events that have a track with this isotope at
any point in time will be persisted.

#### Particle filter

The particle filter does not only affect the output files, but actually works on
the track level. Tracks matching the defined criteria will not be simulated.
With this, it not only reduces output file size, but also reduces the necessary
simulation run time.

```geant4
/RMG/Output/ActivateOutputScheme ParticleFilter
/RMG/Output/ParticleFilter/AddParticle {PDG_CODE}
```

Only particles with the PDG encoding `{PDG_CODE}` will be considered to be
filtered out (the command can be used multiple times). This can be chained with
additional constraints by physical volume in which the particle was created or
by creator process:

- <project:../rmg-commands.md#rmgoutputparticlefilteraddkeepvolume> disables the
  filtering in the specified volume; whereas
  <project:../rmg-commands.md#rmgoutputparticlefilteraddkillvolume> only
  performs the filtering in this volume (the two commands cannot be combined).
- <project:../rmg-commands.md#rmgoutputparticlefilteraddkeepprocess> will only
  keep the specified particles when they were created by the specified
  process(es).
  <project:../rmg-commands.md#rmgoutputparticlefilteraddkillprocess> will not
  simulate the particles when they were created by the specified process (the
  two commands cannot be combined).

Filtering by process and process can be combined.

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

:::{note}

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

This means the columns of the output table are converted from
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
[{edep: [20.1, ..., 74.9], evtid: [0, ..., 0], particle: ..., ...},
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

```
/
├── stp · struct{B00000C,B00000D,...}
│   └── ...
└── tcm · table{row_in_table,table_key}
    ├── row_in_table · array<1>{array<1>{real}}
    └── table_key · array<1>{array<1>{real}}
```

Every row of the TCM corresponds to a simulated event, defined by the same time
window used for reshaping the output tables. For each event, the `table_key`
column specifies the list of detectors that had hits, while the `row_in_table`
column specifies which rows (through the row index) need to be read from the
respective output tables. The detectors are labeled in `row_in_table` by their
UID assigned at registration time (see
{ref}`manual-geometry-register-sens-det`). If output tables are keyed by UID,
querying them based on the TCM data is straightforward. In case Geant4 logical
volume names are used (see {ref}`manual-output-table-naming`), It's still
possible to query data by UID with the symbolic links stored in
`/stp/__by_uid__`, keyed by UID as in the default _remage_ convention.

More information on how to use the TCM is provided in {ref}`manual-analysis`,
while more documentation about how the TCM is generated is available at
{func}`pygama.evt.tcm.generate_tcm_cols`.

## Detector origins

_remage_ stores the global coordinates of each germanium detector in an LH5
struct (or in a table if reshaping is off) called `detector_origins`:

```
/
├── detector_origins · struct{B00000C,B00000D,...}
│   ├── B00000C · struct{xloc,yloc,zloc}
│   │   ├── xloc · real
│   │   ├── yloc · real
│   │   └── zloc · real
│   ├── B00000D · struct{xloc,yloc,zloc}
│   │   ├── xloc · real
│   │   ├── yloc · real
│   │   └── zloc · real
│   └── ...
└── ...
```

:::{note}

For most volume types, the origin is the center of the volume, with some notable
exceptions:

- generic polycones (as used for the detectors in _legend-pygeom-hpges_) have
  their own origin defined which is not at the center.
- for boolean solids, the origin is the same as for the first constituent. With
  subtractions, the origin might even be outside the volume.
- The
  [list in the official documentation](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html)
  contains all rules.

:::

## The vertex table

_remage_ stores data about the simulated event vertex in a table named `vtx`. In
the LH5 output, it can be found in the HDF5 root group (`/vtx`).

```
/
├── stp · struct{B00000C,B00000D,...}
│   └── ...
├── tcm · table{row_in_table,table_key}
│   └── ...
└── vtx · table{evtid,n_part,time,xloc,yloc,zloc}
    ├── evtid · array<1>{real}
    ├── n_part · array<1>{real}
    ├── time · array<1>{real} ── {'units': 'ns'}
    ├── xloc · array<1>{real} ── {'units': 'm'}
    ├── yloc · array<1>{real} ── {'units': 'm'}
    └── zloc · array<1>{real} ── {'units': 'm'}
```

Each row in the table corresponds to a vertex. The columns are:

- `evtid`: Geant4 event identifier
- `time`: time relative to the start of the event (zero, most of the times)
- `n_part`: number of generated particles
- `xloc`, `yloc`, `zloc`: global coordinates of the vertex position.

The table always lists data from all vertices, i.e. the length of the table is
always equal to the number of simulated events.

The vertex table is useful to reconstruct the event vertex of hits recorded in
sensitive detectors by matching the information stored in the `evtid` column.

## The track output scheme

Information about tracks simulated by Geant4 can be often beneficial to
interpret the simulation output. _remage_ is able to store on disk information
about the _initial state_ of all simulated tracks, if instructed to do so with
the command:

```geant4
/RMG/Output/ActivateOutputScheme Track
```

:::{warning}

Optical photon tracks are never written out, since their number in a typical
simulation is often extremely large.

:::

A new table named `tracks` is created in the output file, with columns:

- `evtid`: Geant4 event identifier,
- `time`: track start time relative to the start of the event (zero, most of the
  times),
- `xloc`, `yloc`, `zloc`: global coordinates of the track starting position,
- `px`, `py`, `pz`: momentum of the particle corresponding to the track,
- `ekin`: kinetic energy of the particle corresponding to the track,
- `trackid`: Geant4 numeric identifier of the track,
- `parent_trackid`: Geant4 numeric identifier of the parent track,
- `procid`: numeric identifier of the physical process responsible for the
  creation of the track. A mapping of process names (strings) to these
  identifiers is stored in the `processes` struct,
- `particle`:
  [PDG identifiers](https://pdg.lbl.gov/2025/pdgid/PDGIdentifiers.html) of the
  particle corresponding to the track.

:::{tip}

Use [scikit-hep/particle](https://github.com/scikit-hep/particle) to identify
particles based on their PDG ID.

:::

This is how the data is formatted with LH5 output format enabled:

```text
/
├── tracks · table{ekin,evtid,parent_trackid,particle,procid,px,py,pz,time,trackid,xloc,yloc,zloc}
│   ├── ekin · array<1>{real} ── {'units': 'MeV'}
│   ├── evtid · array<1>{real}
│   ├── parent_trackid · array<1>{real}
│   ├── particle · array<1>{real}
│   ├── procid · array<1>{real}
│   ├── px · array<1>{real} ── {'units': 'MeV'}
│   ├── py · array<1>{real} ── {'units': 'MeV'}
│   ├── pz · array<1>{real} ── {'units': 'MeV'}
│   ├── time · array<1>{real} ── {'units': 'ns'}
│   ├── trackid · array<1>{real}
│   ├── xloc · array<1>{real} ── {'units': 'm'}
│   ├── yloc · array<1>{real} ── {'units': 'm'}
│   └── zloc · array<1>{real} ── {'units': 'm'}
└── processes · struct{compt,eBrem,phot}
    ├── compt · real
    ├── eBrem · real
    ├── ...
    └── phot · real
```

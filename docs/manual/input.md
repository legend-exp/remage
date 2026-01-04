(manual-input)=

# Input event vertices and kinematics

:::{tip}

The python package [_revertex_](https://revertex.readthedocs.io/en/latest/)
contains functionality for generating input files in the correct format.

If you generate the input files on your own without _revertex_, you have to
ensure that all data types and units are as defined here. The input reader does
not support dynamic type conversions, as you would expect for example for python
(i.e. float32 to float64)!

:::

(manual-input-vertex)=

## Vertex input

If the internal vertex confinement options in remage are not sufficient, the
user can instruct _remage_ to load the event vertices from a file written by an
external event generator.

By design, each event can only have a single vertex, that might be shared
between the particles produced in the event.

The input file can be selected with the macro command
<project:../rmg-commands.md#rmggeneratorconfinementfromfilefilename>:

```geant4
/RMG/Generator/Confine FromFile
/RMG/Generator/Confinement/FromFile/FileName {FILE}
```

The vertex input file should be an LH5 table with the following columns:

```
/
└── vtx
    └── vtx · table{xloc,yloc,zloc}
        ├── xloc · array<1>{real} ── {'units': 'm'}
        ├── yloc · array<1>{real} ── {'units': 'm'}
        └── zloc · array<1>{real} ── {'units': 'm'}
```

- `xloc`, `yloc`, `zloc` (double, with units): the global position of the
  particle emission

(manual-input-kinetics)=

## Kinetics input

_remage_ can also read the initial particle information from a file writtem by
an external event generator.

For generating event kinematics we support reading LH5 files with the following
format.

```
/
└── vtx
    └── kin · table{ekin,g4_pid,n_part,px,py,pz}
        ├── ekin · array<1>{real} ── {'units': 'keV'}
        ├── g4_pid · array<1>{real}
        ├── px · array<1>{real}
        ├── py · array<1>{real}
        ├── pz · array<1>{real}
        ├── time · array<1>{real} ── {'units': 'ns'}
        └── n_part · array<1>{real}
```

Here:

- `g4_pid` (int) is the PDG code of the particle (see
  [link](https://pdg.lbl.gov/2007/reviews/montecarlorpp.pdf)),
- `ekin` (double) is the kinetic energy,
- `px`, `py`, `pz` (double) are the momentum direction components (unit vector),
- `time` (double, with units) is global time of the particle emission,
- `n_part` (int) is the number of particles emitted in this event or zero.

Each event starts with a row with `n_part` > 0. This specifies the count of rows
that will be read in for this event. The following rows for this event are then
required to set `n_part` to zero.

Once this input file is created it can be read into _remage_ as an event
generator using the macro command
<project:../rmg-commands.md#rmggeneratorfromfilefilename>:

```geant4
/RMG/Generator/Select FromFile
/RMG/Generator/FromFile/FileName {FILE_PATH}
```

Where `{FILE_PATH}` is the path to the input LH5 file.

## Combining vertex and kinematics input

Combining vertex and kinetics input is possible, even from the same file.
However, by design, each event can only have a single vertex, that will be
shared between all particles for an event from the kinetics file.

:::{warning}

In a multithreaded run, the consistent iteration between vertex and kinetic
input files is not guaranteed. The events from the files might be mixed up in
the simulation, i.e., it is not possible to simulate particle properties
statistically dependent on their location.

:::

:::{tip}

When you want to simulate kinetics from a file at a specific point instead of
confined to volume, you cannot use `/gps/position` to set the position. You can
however use

```geant4
/RMG/Vertex/Select FromPoint
/RMG/Vertex/FromPoint/Position {x} {y} {z}
```

to set a constant position for all events.

:::

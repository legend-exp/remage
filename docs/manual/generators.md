(manual-generators)=

# Event generators

:::{todo}

- list of built-in generators and documentation
- muon generators
  :::

We generate particles of interest using Geant4 macro commands (see [link](https://geant4.web.cern.ch/documentation/dev/bfad_html/ForApplicationDevelopers/GettingStarted/generalParticleSource.html#macro-commands) for some background).

This section of the user manual describes generation of the kinematics of events. However, for some generators
this can also involve generation of positions of the primary particles (for example for muons). In most
other cases the position is handled by the vertex confinement commands descrbed in <project:./confinement.md>.

## General particle source

Most events can be generated using inbuilt Geant4 commands with the General Particle Source (GPS).

These commands allow to generate the position's and kinematics of primary particles. In most
cases we will use in-built _remage_ commands for the positions (see {ref}`manual-confine`).

However, for the kinematics the GPS commands are often sufficient, for example
to generate electrons with a fixed energy and isotropic direction we can use
the macro commands (see [docs](project:../rmg-commands.md#rmggeneratorselect)):

```console
/RMG/Generator/Select GPS
/gps/particle e-
/gps/ang/type iso
```

:::{note}
These commands should be placed after `/run/initialize`
:::

Many other macro commands are available for more complicated generated
[link](https://geant4.web.cern.ch/documentation/dev/bfad_html/ForApplicationDevelopers/GettingStarted/generalParticleSource.html#macro-commands])! Similar commands can be used to generate other primary particles, for example `gamma`, `alpha` or `mu-`.

## Generating decays

A special case of the GPS can be used to generate radioactive decays. This is done
by producing an ion which Geant4 will then handle the decays of and the production of
secondary particles.

The simplest case can be used to generate ions which only undergo a single decay (and
have a stable daughter nuclei). For generate an ion with a given A, Z with the command
`/gps/ion Z A` ([link](https://geant4.web.cern.ch/documentation/dev/bfad_html/ForApplicationDevelopers/GettingStarted/generalParticleSource.html#id7)).

For example we can generate decays of $^{40}$K with:

```console
/RMG/Generator/Select GPS

/gps/particle ion
/gps/energy 0 eV
/gps/ion 19 40
```

:::{note}
In some cases the lifetime of the nuclei is quite long, this would result in the times
in the output files being large, possibly leading to numerical issues. _remage_ will
reset the times to be the time since the start of the decay by default unless the macro
command <project:../rmg-commands.md#rmgprocessessteppingresetinitialdecaytime> is set to
False.
:::

### Decay chains

By default geant4 will propagate the decays of the nuclei until a stable daughter is reached, unless the time
goes beyond the time threshold for radioactive decays, set at $10^{27}$ ns (around $3 \times 10^{10}$ yrs).

:::{warning}
In some cases the lifetime of daughter nuclei can be very long, this can lead
to numerical inaccuracy in the times saved to the output files. _remage_ will warn
you if a track has a global time large enough that the precision is less than 1$\mu$s.
This will occur when times go beyond around 285 yrs.
:::

In some cases, it may be required to simulate only a part of a decay chain. To do this
we can use the `/process/had/rdm/nucleusLimits [aMin] [aMax] [zMin] [zMax]` macro command
[docs](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Fundamentals/biasing.html?highlight=grdm#limited-radionuclides).

This will result in any nuclei having A outside of the range [aMin,aMax] or Z outside of [zMin,zMax] being killed (before having a chance to decay).

For example to generate decays of $^{222}$Rn until $^{210}$Pb we can use the commands:

```console
/RMG/Generator/Select GPS

/gps/particle ion
/gps/energy 0 eV
/gps/ion 86 222
/process/had/rdm/nucleusLimits 214 222 82 86
```

This will allow the decays of $^{222}$Rn (Z=86, A=222) and its daughters:

- $^{218}$Po: (Z=84, A=218),
- $^{214}$Pb: (Z=82, A=214),
- $^{214}$Bi: (Z=83, A=214),
- $^{214}$Po: (Z=84, A=214).

However, the decay of $^{210}$Pb would not be allowed due to the value
of A = 210.

:::{note}
This mechanism can be used to select arbitrary sections of decay chains!
:::

## Double-beta decay physics

To generate double beta decay physics we interface with the _bxdecay0_ package. This requires _remage_
to be build with _bxdecay0_ support see {ref}`install`.
The only documentation for this extension is available [here](https://github.com/BxCppDev/bxdecay0).

The macro [command](project:../rmg-commands.md#rmggeneratorselect):

`/RMG/Generator/Select BxDecay0`

can be used to select the Decay0 generators.

We can then use the _bxdecay_ double beta decay generator macro commands to
generate double beta decay physics. The macro command to generate double
beta decay physics is:

```
/bxdecay0/generator/dbd {ISOTOPE} {SEED} {MODE} {LEVEL}
```

Where:

- ISOTOPE (string): is the decaying nuclei,
- SEED (int): is a random seed,
- MODE (int): is an index the double beta decay mode labelling the decay type,
  a table of implemented decay modes can be found in [link](https://github.com/BxCppDev/bxdecay0/tree/develop?tab=readme-ov-file#list-of-supported-double-beta-decay-modes).
- LEVEL (int): is a index of the energy level of the daughter nuclei, the list of available levels are
  [documented here](https://github.com/BxCppDev/bxdecay0/tree/develop?tab=readme-ov-file#list-of-daughter-nucleus-excited-states-in-double-beta-decay).

For example we can generate two-neutrino double beta decay to the 0+ ground state of $^{76}$Ge with:

```console
/RMG/Generator/Select BxDecay0
/bxdecay0/generator/dbd Ge76 1234 4 0
```

:::{tip}

- More information on the double beta decay commands can be obtained with `/control/manual /bxdecay0/generator/dbd`
- The verbosity of _bxdecay0_ can be increased with `/bxdecay0/generator/verbosity LEVEL` where LEVEL is 0, 1, 2 and 3.
  :::

## Generating events from input files

For more complicated or custom event generators _remage_ supports the possibility
to read in directly event kinematics (or positions) from input files.

The functionality for reading input files is described in <project:./input.md>.
For generating event kinematics we support reading `lh5` files with the following format.

```console
/
└── vtx · HDF5 group
    └── kin · table{px,py,pz,ekin,g4_pid}
        ├── ekin · array<1>{real} ── {'units': 'keV'}
        ├── g4_pid · array<1>{real}
        ├── px · array<1>{real} ── {'units': ''}
        ├── py · array<1>{real} ── {'units': ''}
        └── pz · array<1>{real} ── {'units': ''}

```

Here:

- `ekin` (double) is the kinetic energy,
- `g4_pid` (int) is the particle code (see [link](https://pdg.lbl.gov/2007/reviews/montecarlorpp.pdf)),
- `px,py,pz` (double) are the x,y, and z momenta.

:::{tip}

- It is supported to supply units for the energy with the `lh5` attributes. It is then assumed the
  momenta have the same units!
- The python package _revertex_ [docs](https://revertex.readthedocs.io/en/latest/) contains
  functionality for generating input files in the correct format.
  :::

Once this input file is created it can be read into _remage_ as an event generator using
the macro commands [docs](project:../rmg-commands.md#rmggeneratorfromfilefilename):

```console
/RMG/Generator/Select FromFile
/RMG/Generator/FromFile/FileName {FILE_PATH}
```

Where `{FILE_PATH}` is the path to the input `lh5` file.

:::{warning}
This functionality is currently limited to events where a single primary particle is produced per event.
:::

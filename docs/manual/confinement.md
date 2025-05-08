(manual-confine)=

# Vertex confinement

:::{todo}

- examples
- tricks for vertex visualization

:::

_remage_ supports generating event vertices either in the bulk or on the
surface of various solids. This is essential for simulating, for example, the
decay of radioactive isotopes in detector components.

To enable vertex confinement, activate the corresponding generator using
<project:../rmg-commands.md#rmggeneratorconfine> (see
{ref}`manual-generators`):

```
/RMG/Generator/Confine Volume
```

Configuration is handled via commands in the
<project:../rmg-commands.md#rmggeneratorconfinement> section.

## Listing volumes

The next step is specifying a list of volumes to consider. These can be of two
types: Geant4 physical volumes or user defined virtual geometrical solids.

### Physical volumes

These can be added via
<project:../rmg-commands.md#rmggeneratorconfinementphysicaladdvolume>. To add a
physical volume named e.g. `C000RG1`, do:

```
/RMG/Generator/Confinement/Physical/AddVolume C000RG1
```

`C000RG1` must be part of the provided geometry implementation.
An optional second argument sets the copy number. You can also use regular
expressions (via [`std::regex`](https://en.cppreference.com/w/cpp/regex)) to
match multiple volumes:

```
/RMG/Generator/Confinement/Physical/AddVolume C\w+
```

### Geometrical solids

Virtual user-defined geometrical solids can be defined with commands in the
<project:../rmg-commands.md#rmggeneratorconfinementgeometrical> folder. For
example, to add a sphere of radius 15 cm centered in (-1, 2, 5) cm:

```
/RMG/Generator/Confinement/Geometrical/AddSolid Sphere
/RMG/Generator/Confinement/Geometrical/CenterPositionX -1 cm
/RMG/Generator/Confinement/Geometrical/CenterPositionY 2 cm
/RMG/Generator/Confinement/Geometrical/CenterPositionZ 5 cm
/RMG/Generator/Confinement/Geometrical/Sphere/InnerRadius 0 m
/RMG/Generator/Confinement/Geometrical/Sphere/OuterRadius 15 cm
```

Other shapes, such as boxes and cylinders, are also supported.

## Sampling modes

:::{note}

In this section, sampling in the bulk of volumes is assumed. For documentation
about sampling on surfaces, see next section.

:::

Once a list of solids is defined, we need to select the sampling mode (i.e. how
the list of solids will be used to generate vertices) via
<project:../rmg-commands.md#rmggeneratorconfinementsamplingmode>. Three modes
are currently available:

- `UnionAll` (the default)
- `IntersectPhysicalWithGeometrical`
- `SubtractGeometrical`

In the following, we will refer to physical volumes as $P_i$ and geometrical
volumes as $G_i$.

### `UnionAll`

In this mode, vertices are sampled randomly across all defined solids (physical
and geometrical), _weighted by their volume_ to ensure uniform vertex volume
density. Formally:

```{math}
P_1 \cup \ldots P_n \cup G_1 \cup \ldots \cup G_m
```

### `IntersectPhysicalWithGeometrical`

In this mode, vertices are sampled in the intersection between physical and
geometrical volumes. Formally:

```{math}
(P_1 \cup \ldots P_n) \cap (G_1 \cup \ldots \cup G_m)
```

As for the previous mode, uniform vertex volume density is guaranteed. The
algorithm first samples a candidate vertex from the smaller group (by total
volume, to improve sampling efficiency), then checks if it lies in the other
group. Override the default group with
<project:../rmg-commands.md#rmggeneratorconfinementfirstsamplingvolume>.

### `SubtractGeometrical`

This mode allows to perform subtractions between physical and geometrical
volumes, formally:

```{math}
(P_1 \cup \ldots \cup P_n)
\cap (\overline{G^\prime_1} \cup \ldots \cup \overline{G^\prime_m})
```

Geometrical solids can be added to the group of solids to be subtracted with
the
<project:../rmg-commands.md#rmggeneratorconfinementgeometricaladdexcludedsolid>
command.

:::{note}

The functionality of `IntersectPhysicalWithGeometrical` is still available. If
both physical and geometrical volumes are defined with
<project:../rmg-commands.md#rmggeneratorconfinementphysicaladdvolume>,
the intersection between them will be computed before subtracting the other
solids. Formally:

```{math}
[(P_1 \cup \ldots P_n) \cap (G_1 \cup \ldots \cup G_m)]
 \cap (\overline{G^\prime_1} \cup \ldots \cup \overline{G^\prime_o})
```

:::

## Bulk versus surface

By default, _remage_ will sample vertices in the bulk of solids. Activate sampling
on surfaces with the
<project:../rmg-commands.md#rmggeneratorconfinementsampleonsurface> command.

_remage_ can exactly sample in the bulk or surface of some simple solids
(`G4Box`, `G4Sphere`, `G4Orb` and `G4Tubs`, see
{cpp:func}`RMGGeneratorUtil::rand`). For other volumes, Monte Carlo sampling
methods are implemented. For sampling in the bulk of an arbitrary Geant4 solid,
a rejection-sampling method is implementing by using Geant4's solid extent
and {cpp:func}`G4VSolid::Inside`. For sampling on the surface of an arbitrary
Geant4 solid, the algorithm described in [^1] is implemented.

All sampling modes described above are available, with few notes/limitations:

- The algorithm samples across volumes _weighted by surface area_, to ensure
  uniform vertex surface density.
- Sampling on the surface of volumes that contain daughters will result in
  vertices being distributed on the outer surface of the mother volume _only_.
- Sampling on intersections/subtractions of surfaces is not possible. _remage_
  will instead sample vertices in intersections/subtractions between surfaces
  and volumes. Because of this, the group of volumes (geometrical or physical)
  to sample the candidate surface vertices from must be set with
  <project:../rmg-commands.md#rmggeneratorconfinementfirstsamplingvolume>. This
  is not optional.

## Vertices from an input file

For more complicated vertex confinement _remage_ supports the possibility
to read in directly event primary positions from input files.

The functionality for reading input files is described in <project:./input.md>.
For generating vertices we support reading `lh5` files with the following format.

```console
/
└── vtx · HDF5 group
    └── pos · table{xloc,yloc,zloc}
        ├── xloc · array<1>{real} ── {'units': 'mm'}
        ├── yloc · array<1>{real} ── {'units': 'mm'}
        └── zloc · array<1>{real} ── {'units': 'mm'}
```

:::{note}

- The `lh5` attributes allow to specify the units,
- The positions should be the _global_ coordinates.
- The python package _revertex_ [docs](https://revertex.readthedocs.io/en/latest/) contains
  functionality for generating input files in the correct format.

:::

These vertices can then be read into _remage_ with the macro commands
[docs](project:../rmg-commands.md#rmggeneratorconfinementfromfilefilename):

```console
/RMG/Generator/Confine FromFile
/RMG/Generator/Confinement/FromFile/FileName {FILE_PATH}
```

Where {FILE_PATH} is the path to the input file.

[^1]:
    J. A. Detwiler, R. Henning, R. A. Johnson and M. G. Marino, in IEEE
    Transactions on Nuclear Science, vol. 55, no. 4, pp. 2329-2333, Aug. 2008,
    doi: [10.1109/TNS.2008.2001063](https://doi.org/10.1109/TNS.2008.2001063)

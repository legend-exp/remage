# Vertex confinement

This section of the validation suite describes the generation of primary event vertices and their confinement within a particular geometrical or
physical volume.

These tests consist both of Geant4 visualisation plots showing the location of generated primaries as well as some statistical tests.

## Natively sampleable solids

The first set of tests are related to sampling on the bulk or the surface of simple solids, these can be sampled natively (i.e. without rejection sampling).

```{figure} ./_img/confinement/native-volume.output.jpeg
:width: 500px
Vertices sampled in the bulk of solids.
```

&nbsp;

```{figure} ./_img/confinement/native-surface.output.jpeg
:width: 500px
Vertices sampled on the surface of solids.
```

&nbsp;

You should see that the primaries are generated in the cylinder (far left), box (front center), orb (back and center) and also
for the bulk simulation in the sphere section (front). It should also be possible to appreciate an apparent higher density of
points for the surface simulation.

## Complex solids

The next tests relate to generation of points on complex solids, generated from intersections and unions.
Similar to the previous test simulations are performed both for the surface and the bulk.

```{figure} ./_img/confinement/complex-volume.output.jpeg
:width: 500px
Vertices sampled in the bulk of solids.
```

&nbsp;

```{figure} ./_img/confinement/complex-surface.output.jpeg
:width: 500px
Vertices sampled on the surface of solids.
```

&nbsp;

You should see points generated on the polycone (back left), in a box with a center cut out (directly in front of the polycone), in a solid formed of the intersection
of a box and sphere (back right). For the bulk simulation you should also see points generated in the sphere with a box inside (front right). Again you should be able
to appreciate a higher density of points on the surface in the surface simulation.

## Unions and intersections

remage also has the possibility to generate primaries in a geometrical volume, i.e. a user defined shape not necessarily corresponding to any physical
volume in the detector. In addition, there is the possibility to select vertices just in the intersection of a physical and geometrical volume or in the union
of some physical and geometrical volumes. This subsection validates these features.

```{figure} ./_img/confinement/geometrical.output.jpeg
:width: 500px
Vertices sampled in geometrical (user defined) volumes
```

&nbsp;

```{figure} ./_img/confinement/geometrical-or-physical.output.jpeg
:width: 500px
Vertices sampled in geometrical (user defined) or physical volumes.
```

&nbsp;

```{figure} ./_img/confinement/geometrical-and-physical.output.jpeg
:width: 500px
Vertices sampled in the intersection of geometrical and physical volumes.
```

&nbsp;

- In the first case, you should see points sampled in three user defined volumes, a section of a cylinder (far left),
  a box (center back) and a sphere (center front - overlapping with a box).
- In the second plot, the primaries are generated eithert in the sphere (from the previous section), or a box (back right), or the small section of a cylinder (front center).
- Finally in the last case primaries should be generated in the intersection of the sphere (front right) with a user defined smaller sphere.

## Generic surface sampling

_remage_ has a dedicated algorithm for sampling points on the surface of any solid. The next section of the
validation report provides tests on the intermediate steps of this algorithm and on the final result.

### Bounding sphere

The surface sampling algorithm requires a bounding sphere containing the solid. Points are then generated outside this bounding sphere, and
based on a random direction the number of intersections is computed.

The following figures show the bounding sphere and initial points (which should be outside the bounding sphere). You should see that the
grey bounding spheres are large enough to contain the green solids and that the red points all lie outside the grey box.

```{figure} ./_img/confinement/surface-sample-bounding-box-simple.output.jpeg
:width: 500px
Bounding sphere for surface sampling with a simple solid (cylinder).
```

&nbsp;

```{figure} ./_img/confinement/surface-sample-bounding-box-subtraction.output.jpeg
:width: 500px
Bounding sphere for surface sampling with a subtraction based solid.
```

&nbsp;

```{figure} ./_img/confinement/surface-sample-bounding-box-union.output.jpeg
:width: 500px
Bounding sphere for surface sampling with a union (of two boxes) based solid.
```

&nbsp;

### Location of sampled vertices

The next figures show the location of generated primaries for various different solids. You should see the
primaries are contained in the appropriate solid and that they are distributed on the surface.

```{figure} ./_img/confinement/vis-surface-tubby.output.jpeg
:width: 500px
Sampled vertices for surface generation on a cylinder (shown in light pink).
```

&nbsp;

```{figure} ./_img/confinement/vis-surface-tubby.output.jpeg
:width: 500px
Sampled vertices for surface generation on a box (shown in green).
```

&nbsp;

```{figure} ./_img/confinement/vis-surface-trd.output.jpeg
:width: 500px
Sampled vertices for surface generation on a trapezoid (shown in purple).
```

&nbsp;

```{figure} ./_img/confinement/vis-surface-uni.output.jpeg
:width: 500px
Sampled vertices for surface generation on the union of two boxes (shown in red).
```

&nbsp;

```{figure} ./_img/confinement/vis-surface-sub.output.jpeg
:width: 500px
Sampled vertices for surface generation on the subtraction of two cylinders (shown in dark pink).
```

&nbsp;

```{figure} ./_img/confinement/vis-surface-con.output.jpeg
:width: 500px
Sampled vertices for surface generation on the subtraction of a cone (shown in dark blue).
```

&nbsp;

## Statistical tests of uniformity

## Dedicated simulations for Ge in LAr

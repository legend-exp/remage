# Vertex confinement

This section of the validation suite describes the generation of primary event vertices and their confinement within a particular geometrical or
physical volume.

These tests consist both of Geant4 visualisation plots showing the location of generated primaries as well as some statistical tests.

## Natively sampleable solids

The first set of tests are related to sampling on the bulk or the surface of simple solids, these can be sampled natively (i.e. without rejection sampling).

:::{subfigure} AB
:subcaptions: above

```{image} ./_img/confinement/native-volume.output.jpeg
:height: 400px
:alt: Vertices sampled in the bulk of solids.
```

```{image} ./_img/confinement/native-surface.output.jpeg
:height: 400px
:alt: Vertices sampled on the surface of solids.
```

Vertices sampled in simple solids.

:::

&nbsp;

You should see that the primaries are generated in the cylinder (far left), box (front center), orb (back and center) and also
for the bulk simulation in the sphere section (front). It should also be possible to appreciate an apparent higher density of
points for the surface simulation.

## Complex solids

The next tests relate to generation of points on complex solids, generated from intersections and unions.
Similar to the previous test simulations are performed both for the surface and the bulk.

:::{subfigure} AB
:subcaptions: above

```{image} ./_img/confinement/complex-volume.output.jpeg
:height: 400px
:alt: Vertices sampled in the bulk of solids.
```

```{image} ./_img/confinement/complex-surface.output.jpeg
:height: 400px
:alt: Vertices sampled on the surface of solids.
```

Vertices sample in complex solids.

:::

&nbsp;

You should see points generated on the polycone (back left), in a box with a center cut out (directly in front of the polycone), in a solid formed of the intersection
of a box and sphere (back right). For the bulk simulation you should also see points generated in the sphere with a box inside (front right). Again you should be able
to appreciate a higher density of points on the surface in the surface simulation.

## Unions and intersections

remage also has the possibility to generate primaries in a geometrical volume, i.e. a user defined shape not necessarily corresponding to any physical
volume in the detector. In addition, there is the possibility to select vertices just in the intersection of a physical and geometrical volume or in the union
of some physical and geometrical volumes. This subsection validates these features.

:::{subfigure} ABC
:subcaptions: above

```{image} ./_img/confinement/geometrical.output.jpeg
:height: 200px
:alt: Geometrical (user defined) volumes.
```

```{image} ./_img/confinement/geometrical-or-physical.output.jpeg
:height: 200px
:alt: Geometrical (user defined) or physical volumes.
```

```{image} ./_img/confinement/geometrical-and-physical.output.jpeg
:height: 200px
:alt: Intersection of geometrical and physical volumes.
```

Checks on sampling for intersections and unions.
:::

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

:::{subfigure} ABC
:subcaptions: above

```{image} ./_img/confinement/surface-sample-bounding-box-simple.output.jpeg
:width: 200px
:alt: Bounding sphere for cylinder.
```

```{image} ./_img/confinement/surface-sample-bounding-box-subtraction.output.jpeg
:width: 200px
:alt: Bounding sphere for subtraction.
```

```{image} ./_img/confinement/surface-sample-bounding-box-union.output.jpeg
:width: 200px
:alt: Bounding sphere for union.
```

Checks on the bounding spheres.
:::

### Location of sampled vertices

The next figures show the location of generated primaries for various different solids. You should see the
primaries are contained in the appropriate solid and that they are distributed on the surface.

:::{subfigure} AB|CD|EF
:subcaptions: above

```{image} ./_img/confinement/vis-surface-tubby.output.jpeg
:width: 300px
:alt: Sampled on a cylinder (shown in light pink).
```

```{image} ./_img/confinement/vis-surface-box.output.jpeg
:width: 300px
:alt: Sampled on a box (shown in green).
```

```{image} ./_img/confinement/vis-surface-trd.output.jpeg
:width: 300px
:alt: Sampled on a trapezoid (shown in purple).
```

```{image} ./_img/confinement/vis-surface-uni.output.jpeg
:width: 300px
:alt: Sampled on the union of two boxes (shown in red).
```

```{image} ./_img/confinement/vis-surface-sub.output.jpeg
:width: 300px
:alt: Sampled on the subtraction of two cylinders (shown in dark pink).
```

```{image} ./_img/confinement/vis-surface-con.output.jpeg
:width: 300px
:alt: Sampled vertices on a cone (shown in dark blue).
```

Checks on the location of vertices for surface sampling.
:::

### Statistical tests of uniformity

The final part of the test for the generic surface sampler is a series of statistical tests on the uniformity of the points on
the surface of the shapes. This is based on counting the number of primaries on each face of the solids and comparing the ratio
to that of the surface areas. The tests are based on the same simulation as in the previous section.

First we plot the 3D position of the vertices labelling which side the vertex is closest too by color. If a vertex is not
close to any side the test will fail.

:::{subfigure} AB|CD|EF
:subcaptions: above

```{image} ./_img/confinement/confinement.simple-solids-surface-tubby-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-box-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-trd-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-uni-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-sub-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-con-3d.output.png
:width: 400px
```

:::
You should be able to verify that the primaries close to each surface are correctly identified
and that the primaries are indeed distributed on a surface.

Finally, we perform the statistical tests comparing the ratio of primaries on each surface
to the ratio of surface area. You should see only statistical fluctuations.

:::{subfigure} AB|CD|EF
:subcaptions: above

```{image} ./_img/confinement/confinement.simple-solids-surface-tubby-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-box-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-trd-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-uni-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-sub-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-con-ratios.output.png
:width: 400px
```

:::

## Dedicated simulations for Ge in LAr

The last sub-section of this part of the validation report contains a more specific test with an array
of HPGe detectors emersed in LAr. The geometry is shown below, showing the HPGe detectors (blue) in
the red LAr cylinder.

```{figure} ./_img/confinement/vis-ge-array.output.jpeg
:width: 400px
:alt: Geant4 visualisation of a HPGe array in LAr.
```

### Relative fraction of events per HPGe volume

The first test consists of generating events uniformly across all the HPGe detectors and checking
that the fraction of primaries in each volume matches the ratio of the volumes.

```{figure} ./_img/ confinement/relative-ge.output.png
:width: 700px
```

### Generation in parts of the LAr volume

We generate events in intersection of the LAr volume surrounding the detectors and a cylinder
around each detector string. We plot a histogram of the positions of the primaries
which should show gaps due to the HPGe detectors. And we compute the ratio of primaries
in each cylinder, which should be proportional to the volume of the cylinder minus that
of the Germanium.

```{figure} ./_img/ confinement/lar-in-check-xy.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-in-check-xz.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-in-check-ratios.output.png
:width: 700px
```

Finally, we generate primaries in the subtraction of the LAr region and the cylinders around each string.

```{figure} ./_img/ confinement/lar-sub-check-xy.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-sub-check-xz.output.png
:width: 700px
```

Then also with an intersection of another cylinder with the height of the strings.

```{figure} ./_img/ confinement/lar-int-and-sub-check-xy.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-int-and-sub-check-xz.output.png
:width: 700px
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

```{image} ./_img/confinement/geometrical.output.jpeg
:height: 200px
:alt: Geometrical (user defined) volumes.
```

```{image} ./_img/confinement/geometrical-or-physical.output.jpeg
:height: 200px
:alt: Geometrical (user defined) or physical volumes.
```

```{image} ./_img/confinement/geometrical-and-physical.output.jpeg
:height: 200px
:alt: Intersection of geometrical and physical volumes.
```

Checks on sampling for intersections and unions.
:::

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

:::{subfigure} ABC
:subcaptions: above

```{image} ./_img/confinement/surface-sample-bounding-box-simple.output.jpeg
:width: 200px
:alt: Bounding sphere for cylinder.
```

```{image} ./_img/confinement/surface-sample-bounding-box-subtraction.output.jpeg
:width: 200px
:alt: Bounding sphere for subtraction.
```

```{image} ./_img/confinement/surface-sample-bounding-box-union.output.jpeg
:width: 200px
:alt: Bounding sphere for union.
```

Checks on the bounding spheres.
:::

### Location of sampled vertices

The next figures show the location of generated primaries for various different solids. You should see the
primaries are contained in the appropriate solid and that they are distributed on the surface.

:::{subfigure} AB|CD|EF
:subcaptions: above

```{image} ./_img/confinement/vis-surface-tubby.output.jpeg
:width: 300px
:alt: Sampled on a cylinder (shown in light pink).
```

```{image} ./_img/confinement/vis-surface-box.output.jpeg
:width: 300px
:alt: Sampled on a box (shown in green).
```

```{image} ./_img/confinement/vis-surface-trd.output.jpeg
:width: 300px
:alt: Sampled on a trapezoid (shown in purple).
```

```{image} ./_img/confinement/vis-surface-uni.output.jpeg
:width: 300px
:alt: Sampled on the union of two boxes (shown in red).
```

```{image} ./_img/confinement/vis-surface-sub.output.jpeg
:width: 300px
:alt: Sampled on the subtraction of two cylinders (shown in dark pink).
```

```{image} ./_img/confinement/vis-surface-con.output.jpeg
:width: 300px
:alt: Sampled vertices on a cone (shown in dark blue).
```

Checks on the location of vertices for surface sampling.
:::

### Statistical tests of uniformity

The final part of the test for the generic surface sampler is a series of statistical tests on the uniformity of the points on
the surface of the shapes. This is based on counting the number of primaries on each face of the solids and comparing the ratio
to that of the surface areas. The tests are based on the same simulation as in the previous section.

First we plot the 3D position of the vertices labelling which side the vertex is closest too by color. If a vertex is not
close to any side the test will fail.

:::{subfigure} AB|CD|EF
:subcaptions: above

```{image} ./_img/confinement/confinement.simple-solids-surface-tubby-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-box-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-trd-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-uni-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-sub-3d.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-con-3d.output.png
:width: 400px
```

:::
You should be able to verify that the primaries close to each surface are correctly identified
and that the primaries are indeed distributed on a surface.

Finally, we perform the statistical tests comparing the ratio of primaries on each surface
to the ratio of surface area. You should see only statistical fluctuations.

:::{subfigure} AB|CD|EF
:subcaptions: above

```{image} ./_img/confinement/confinement.simple-solids-surface-tubby-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-box-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-trd-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-uni-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-sub-ratios.output.png
:width: 400px
```

```{image} ./_img/confinement/confinement.simple-solids-surface-con-ratios.output.png
:width: 400px
```

:::

## Dedicated simulations for Ge in LAr

The last sub-section of this part of the validation report contains a more specific test with an array
of HPGe detectors emersed in LAr.

### Relative fraction of events per HPGe volume

The first test consists of generating events uniformly across all the HPGe detectors and checking
that the fraction of primaries in each volume matches the ratio of the volumes.

```{figure} ./_img/ confinement/relative-ge.output.png
:width: 700px
```

### Generation in parts of the LAr volume

We generate events in intersection of the LAr volume surrounding the detectors and a cylinder
around each detector string. We plot a histogram of the positions of the primaries
which should show gaps due to the HPGe detectors. And we compute the ratio of primaries
in each cylinder, which should be proportional to the volume of the cylinder minus that
of the Germanium.

```{figure} ./_img/ confinement/lar-in-check-xy.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-in-check-xz.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-in-check-ratios.output.png
:width: 700px
```

Finally, we generate primaries in the subtraction of the LAr region and the cylinders around each string.

```{figure} ./_img/ confinement/lar-sub-check-xy.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-sub-check-xz.output.png
:width: 700px
```

Then also with an intersection of another cylinder with the height of the strings.

```{figure} ./_img/ confinement/lar-int-and-sub-check-xy.output.png
:width: 700px
```

```{figure} ./_img/ confinement/lar-int-and-sub-check-xz.output.png
:width: 700px
```

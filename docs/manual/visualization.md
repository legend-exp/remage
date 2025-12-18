# Visualization

## The `legend-pygeom-vis` viewer

For simple geometry visualizatio without particle tracks, it is recommended to
use `legend-pygeom-vis` from the
[_legend-pygeom-tools_](https://github.com/legend-exp/legend-pygeom-tools)
package or the builtin visualization capabilities of your geometry creation
tool. This viewer also supports adding points from a LH5 file as an overlay over
the geometry.

This VTK-based viewer can be run on any system, and _remage_ must not even be
installed on that device. In our experience, it might be easier to set up than
the Geant4-based visualization.

## Built-in Geant4 visualization

For more complex visualization, i.e., when you need to visualize the full
simulated tracks, you have to use the builtin visualization capabilities of
Geant4. For this it is useful to consult the [upstream visualization docs]. Only
the visualization backends compiled into your Geant4 distribution are available
at runtime. For typical use cases, the OpenGL and ToolsSG drivers should be
sufficient. Whereas ToolsSG is suitable for offscreen rendering (i.e. on CI
pipelines), the OpenGL drivers typically require a running X11 session.

Unlike the `legend-pygeom-vis` viewer, Geant4 cannot read the colors from the
GDML file, but requires expplicit color definition with macro commands. However,
such a coloring macro can also typically be generated from your GDML file
creation tool.

The basic setup of a visualization macro needs to contain some commands to setup
the desired viewer to contain the geometry and the particle tracks:

```geant4
/vis/open OGL  # or some other driver.
/vis/sceneHandler/attach

/vis/drawVolume {root_volume_to_draw}

/vis/viewer/set/defaultColour black
/vis/viewer/set/background white

/vis/viewer/set/upVector 0 0 -1
/vis/viewer/set/viewpointVector 1 1 0.5
/vis/viewer/set/rotationStyle freeRotation

/vis/scene/add/trajectories smooth
/vis/scene/add/hits
/vis/scene/endOfEventAction accumulate
# more commands...
/vis/viewer/flush
```

Individual volumes can be colored/shown/hidden...:

```geant4
# draw volume {volume} as a solid instead of wireframe.
/vis/geometry/set/forceSolid {volume}
# set the volume color of {volume}
/vis/geometry/set/colour {volume} 0 {r} {g} {b} {a}
# hide the volume {volume}.
/vis/geometry/set/visibility {volume} -1 false
```

The Geant4 visualization can also color tracks by properties of the particles.
The following example macro illustrates this in the case of optical photons,
colored by their wavelength.

````{admonition} coloring photon tracks by wavelength
:class: dropdown

```geant4
/vis/modeling/trajectories/create/drawByAttribute
/vis/modeling/trajectories/select drawByAttribute-0
/vis/modeling/trajectories/drawByAttribute-0/verbose false
/vis/modeling/trajectories/drawByAttribute-0/setAttribute IMag

/vis/modeling/trajectories/drawByAttribute-0/addInterval vuviolet 6.53 eV 12.4 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval duviolet 4.43 eV 6.53 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval muviolet 3.94 eV 4.43 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval nuviolet 3.26 eV 3.94 eV

/vis/modeling/trajectories/drawByAttribute-0/addInterval violet  2.85 eV 3.26 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval blue    2.48 eV 2.85 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval cyan    2.38 eV 2.48 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval green   2.19 eV 2.38 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval yellow  2.10 eV 2.19 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval orange  1.98 eV 2.10 eV
/vis/modeling/trajectories/drawByAttribute-0/addInterval red     1.59 eV 1.98 eV

/vis/modeling/trajectories/drawByAttribute-0/vuviolet/setLineColourRGBA 0.29 0.00 0.51 1
/vis/modeling/trajectories/drawByAttribute-0/duviolet/setLineColourRGBA 0.50 0.00 0.50 1
/vis/modeling/trajectories/drawByAttribute-0/muviolet/setLineColourRGBA 0.58 0.00 0.83 1
/vis/modeling/trajectories/drawByAttribute-0/nuviolet/setLineColourRGBA 0.73 0.33 0.83 1

/vis/modeling/trajectories/drawByAttribute-0/violet/setLineColourRGBA  0.93 0.51 0.93 1
/vis/modeling/trajectories/drawByAttribute-0/blue/setLineColourRGBA    0.00 0.00 1.00 1
/vis/modeling/trajectories/drawByAttribute-0/cyan/setLineColourRGBA    0.00 1.00 1.00 1
/vis/modeling/trajectories/drawByAttribute-0/green/setLineColourRGBA   0.00 1.00 0.00 1
/vis/modeling/trajectories/drawByAttribute-0/yellow/setLineColourRGBA  1.00 1.00 0.00 1
/vis/modeling/trajectories/drawByAttribute-0/orange/setLineColourRGBA  1.00 0.65 0.00 1
/vis/modeling/trajectories/drawByAttribute-0/red/setLineColourRGBA     1.00 0.00 0.00 1
```

````

### Offscreen rendering with ToolsSG

Using the ToolsSG offscreen rendering to render to a file is easy:

```geant4
# after /run/initialize
/vis/open TOOLSSG_OFFSCREEN 1500x1500

# your simulation commands here...
# /run/beamOn ...

/vis/tsg/offscreen/set/file {filename}.jpeg
/vis/viewer/rebuild
```

## Interactive visualization

The interactive mode `remage -i` can be used together with the visualization and
allows to input commands in a GUI window. There is no way to run a macro at
startup in this window, however.

Environment variables can be used to force a specific (G)UI toolkit, for example
`G4UI_USE_TCSH=1` to force a console UI, or `G4UI_USE_QT=1` to force a QT
interface.

[upstream visualization docs]:
  https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Visualization/visualization.html

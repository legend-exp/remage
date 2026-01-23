# Neutron transport and capture spectrum

Here we investigate the neutron capture physics. For this, neutrons are
simulated in a steel container filled with liquid argon. The steel container is
located within a water volume and inside of the argon there is also a neutron
moderator, which consists of hydrogen and is loaded with gadolinium. The goal of
the geometry is to enable not only neutron capture on different isotopes, but
also have the neutron transport be a crucial component regarding capture
distribution. The main elements on which can be captured in this geometry are
therefore H, Ar, Cr, Fe, Ni, Gd.

The neutron transport and capture cross-sections might not be compared to
literature values, but they can be compared in between _remage_ versions. Every
plot also shows the number of neutrons captured on this isotope in the setup. If
this number considerably changes between _remage_ versions, a change in either
the neutron transport or the neutron capture cross-section can be assumed.

The neutron capture process can be validated against literature values. Hydrogen
has a well-known singular gamma line at 2.22 keV after a neutron capture, making
bugs very easy to spot. Gadolinium on the contrary, has a very complex gamma
line intensity, which can be compared to
[publications](https://link.springer.com/article/10.1140/epjc/s10052-023-11602-y).
The Q-Values of the neutron capture process is well known for all captures in
this setup. The expected Q-value according to the
[IAEA Nuclear Data Section](https://www-nds.iaea.org/) is shown in each plot for
the respective isotope. This can directly be compared with the orange line,
depicting the summed energy of all secondary particles resulting from the
neutron capture process in Geant4. In blue, the gamma line intensity is shown.

For every isotope there are always two plots shown. The plot on the left just
specifies the basic `G4Shielding` physicslist to the simulation without any
extra modifications. The plot on the right instead additionally forces the
`G4PhotonEvaporation` model to be used by specifying the macro command
`/process/had/particle_hp/use_photo_evaporation true`. The `G4PhotonEvaporation`
model should always correctly respect the Q-value of the capture, but is very
likely to not correctly represent the gamma line intensity. The standard
`G4Shielding` option might fall back to the `G4PhotonEvaporation` depending on
the available data and version. Both plots might show a different number of
captures per isotope due to the random engine getting out of sync. Still numbers
should be within standard deviation of another.

Only these two models are shown due to run-time constraints. Still, many
isotopes will show lacking statistics as simulation times have to remain within
reason for a CI pipeline.

```{figure} ./_img/neutrons/neutron_capture_H_1.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Ar_36.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Ar_38.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Ar_40.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Cr_50.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Cr_52.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Cr_53.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Fe_54.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Fe_56.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Fe_57.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Ni_58.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Ni_60.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Ni_62.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Gd_155.output.png
:width: 800px
```

```{figure} ./_img/neutrons/neutron_capture_Gd_157.output.png
:width: 800px
```

:::{note}

The orange line is not only a sum of the gamma energies, but also includes any
energy contribution from the recoil nucleus and internal conversion electrons.
It includes ANY secondary produced in the Geant4 process and should correctly
represent the Q-value. The plot label is just for convenience.

:::

:::{note}

Simulation and analysis scripts are available in
[`tests/neutrons`](https://github.com/legend-exp/remage/tree/main/tests/neutrons).

:::

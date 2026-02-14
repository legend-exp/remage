# Optical physics

In this section we compare the _remage_ simulation output of optical processes
with their theoretical expectations.

## Scintillation emission

Different particles of varying kinetic energies are started in the center of a
sphere of scintillating liquid argon. No attenuating optical processes are
enabled. Cerenkov emission is disabled. The number and energy of the created
photons is recorded.

For all particles, the same emission peak around a wavelength of 128 nm is
expected. For each primary energy, a narrow peak of emitted photons is visible,
varying with particle type.

```{subfigure} AA|BC
:subcaptions: above

:::{image} ./_img/optics/scintillation-e-.output.png
:width: 50%
:alt: Light output for electrons of various kinetic energies.
:::

:::{image} ./_img/optics/scintillation-alpha.output.png
:width: 97%
:alt: Light output for alpha particles of various kinetic energies.
:::

:::{image} ./_img/optics/scintillation-ion.output.png
:width: 97%
:alt: Light output for ions (C-12) of various kinetic energies.
:::

Light output for various particles and kinetic energies.
```

The light output for each particle forms a linear function. In the Geant4
simulation, the scintillation yield is implemented with a particle-type
dependent linear function. The expectation is also shown in the plot.

```{figure} ./_img/optics/scintillation-yield.output.png
:width: 70%
:alt: The light output for each particle forms a linear function.
```

## Cerenkov emission

Electrons of varying kinetic energies are started in the center of a sphere of
water. Optical photon attenuation is disabled. Scintillation emission is
disabled. The number and energy of the created photons is recorded.

The emitted photon spectrum is equal for all electron energies after
normalization to the total number of emitted photons, and not mono-energetic (as
expected).

```{subfigure} AA|BB
:subcaptions: above

:::{image} ./_img/optics/cerenkov-e-.output.png
:width: 50%
:alt: Electron integrated path lengths versus initial electron energy.
:::

:::{image} ./_img/optics/cerenkov-yield.output.png
:width: 70%
:alt: Light output for electrons in water
:::

Cerenkov light output for electrons of varying kinetic energies in water.
```

:::{important}

The energy range of emitted Cerenkov photons is limited to the range of the
`RINDEX` optical property of the material!

:::

## Light detection (surface physics)

This validation test checks the behaviour at a surface that should detect
photons. The setup is as following:

- a registered sensitive volume in a sphere of liquid argon.
- an optical surface around that volume, with constant material properties
  `EFFICIENCY` and `REFLECTIVITY`.
- only the liquid argon has a refractive index set, no other (attenuating)
  material properties.

Monoenergetic optical photons are started above the detecting surface and the
number of detected hits is recorded. Between the different runs, the values of
the surfaces efficiency and reflectivity are changed.

It is expected, that the total number of detected photons is proportional to the
detection `EFFICIENCY` and also proportional to `1 - REFLECTIVITY`:

```{figure} ./_img/optics/photon-detection.output.png
:width: 70%
:alt: Detected light at a surface with efficiency and reflectivity
```

:::{note}

Simulation and analysis scripts are available in
[`tests/optics`](https://github.com/legend-exp/remage/tree/main/tests/optics).

:::

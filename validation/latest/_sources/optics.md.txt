# Optical physics

In this section we compare the _remage_ simulation output of optical processes
with their theoretical expectations.

## Scintillation emission

Different particles of varying kinetic energies are started in the center of a
sphere of scintillating liquid argon. No attenuating optical processes are
enabled. Cerenkov emission is disabled. The number and energy of the created
photons is recorded.

For all particles, the same emission spectrum around a wavelength of 128 nm is
expected. We do not expect the typical narrow liquid argon emission peak here,
as for this validation we implemented a broad shape (step-function) as the
emission spetctrum[^spectral_densities]. For each primary energy, a narrow peak
of emitted photons is visible, varying with particle type.

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

The light output for each particle forms a linear function.
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

Detected light at a surface with efficiency and reflectivity
```

## Attenuation (Absorption & Rayleigh scattering)

This validation test checks the behaviour of the attenuating processes
implemented in Geant4. These processes include bulk absorption through the
`ABSLENGTH` material property and Rayleigh scattering through the `RAYLEIGH`
property. The total attenuation is observed as the combination of both processes
that each have an exponential profile.

The geometry consists of a registered sensitive volume at one end of a tube of
liquid argon (with 100% efficiency and 0% reflectivity). Optical photons are
started in various distances in from of the detector plane, perpendicular to the
detetcor plane.

For each length $l \in {30\mathrm{ cm}, 60\mathrm{ cm}}$, three different sets
of optical properties are simulated:

- setting only an absorption length of $l$ and no Rayleigh scattering,
- setting only a Rayleigh scattering length of $l$ and no bulk absorption, and
- setting both, so that $1/l = 1/\lambda_{abs} + 1/\lambda_{rayl}$ is fulfilled.

```{subfigure} AB
:subcaptions: above

:::{image} ./_img/optics/photon-attenuation-30.output.png
:width: 97%
:alt: Attenuation behaviour for l = 30 cm.
:::

:::{image} ./_img/optics/photon-attenuation-60.output.png
:width: 97%
:alt: Attenuation behaviour for l = 60 cm.
:::

Attenuation behaviour in liquid argon with simulated points and fitted
exponential attenuation curves.
```

The absorption-only variant shows a clean fit of the expected attenuation
length. In this simple case (without scattering), the absorption is fully
linear. For the Rayleigh-only case, the fitted attenuation length is larger.
This should be an effect of the finite aperture of the detector. Some
forward-scattered photons will still reach the detecting surface, despite being
scattered. The mixed case shows a behaviour in between the two cases.

## Wavelenth-shifting (WLS)

The simulation setup contains a volume of liquid argon as optical medium. The
attenuating properties of the liquid argon have been removed, and a thin disk of
a wavelength-shifting material is introduced between emission point and
detector. The WLS emission is isotropic, so the detector is a sphereical surface
that contains the shifting disk and a small opening for the incoming light, to
also be able to detect back-scattered shifted light.

To reduce the effect of the border interfaces to the WLS disk, the disk has the
same refractive index as the LAr. It is also not a known material from LEGEND,
to simplify the calculation of the expectation values[^tpb_note]. It has a WLS
absorption length of 2 mm and a broad emission between around 200 and 300 nm.

The simulation is repeated with varying thickness of the WLS disk. VUV photons
of 128 nm wavelength are started; the detector records the photons with their
wavelength, and also their arrival times.

```{figure} ./_img/optics/photon-wls.output.png
:width: 70%

Detected light after transmission through a WLS disk.
```

With increasing thickness of the WLS disk, the fraction of shifted light
increases exponentially, and the fraction of transmitted VUV light decreases.

### Time profiles of wavelength shifting

Geant4 offers two modes how the WLS time profiles are generated. The default
mode, `delta`, does not sample from the (exponential= emission time distribution
characterized by the WLS time constant, but assumes a delta-function as time
distribution. Only the `exponential` mode will sample from the correct
distribution:

```{subfigure} AB
:subcaptions: above

:::{image} ./_img/optics/photon-wls-times-delta.output.png
:width: 97%
:alt: Recorded photon arrival times for "delta" time profile.
:::

:::{image} ./_img/optics/photon-wls-times-exponential.output.png
:width: 97%
:alt: Recorded photon arrival times for "exponential" time profile.
:::

Recorded photon arrival times after WLS.
```

### Number of emitted WLS photons

The number of emitted photons can be influenced with the scalar material
property `WLSMEANNUMBERPHOTONS`. _remage_ implements a custom WLS process to
limit the number of emitted photons to 1, instead of lsampling from a Poisson
distribution as in stock Geant4. The effect of this process can be seen here:
With `RMGOpWLS`, this limit is enforced, whereas with `OpWLS`, it is not.

```{figure} ./_img/optics/photon-wls-photon-count.output.png
:width: 70%

Number of emitted photons from the `{RMG,}OpWLS` processes.
```

:::{note}

Simulation and analysis scripts are available in
[`tests/optics`](https://github.com/legend-exp/remage/tree/main/tests/optics).

:::

[^spectral_densities]:
    For the emission quantities of scintillation and WLS processes, using a flat
    function makes it easier to spot deviations in sampling.

[^tpb_note]:
    The most prominent WLS material, TPB, has complex optical properties. It has
    an extremely short WLS absorption length and also has overlapping absorption
    and emission spectra.

# Comparison with NIST

In this section we compare the _remage_ simulation output with theoretical
predictions from the [NIST](https://www.nist.gov) database.

## Electron interactions

We calculate stopping power and integrated path length of the simulated
electrons and compare them with data from the
[ESTAR database](https://physics.nist.gov/PhysRefData/Star/Text/ESTAR.html) (a
short description of the reported quantities is available
[here](https://physics.nist.gov/PhysRefData/Star/Text/method.html)).

The simulation physics is modified by the commands:

```
/process/inactivate msc
/RMG/Processes/DefaultProductionCut 1 km
/RMG/Processes/SensitiveProductionCut 1 km
```

- Multiple Coulomb scattering is disabled, since it prevents us to measure the
  true integrated particle path length in the simulation (unless a very short
  step size limit is set).
- the high production cuts ensure that no secondaries are created. This is
  required to correctly measure the bremsstrahlung component of the CSDA loss.

An effect of the step length is visible. The larger the step length, the larger
the difference to the expected integrated path length.

The statistical uncertainties are dominated by energy-loss fluctuations. They
are kept enabled here to show the effect of the user-chosen step length values.
Especially the stopping power at low step sizes are the most affected.

### Electron interactions in natural germanium

Monoenergetic electrons are simulated in a natural germanium sphere of 15 cm
radius.

```{subfigure} AA|BC
:subcaptions: above

:::{image} ./_img/nist/e-range-ge-vs-estar.output.png
:width: 50%
:alt: Electron integrated path lengths versus initial electron energy.
:::

:::{image} ./_img/nist/e-range-ge-distributions.output.png
:width: 70%
:alt: Distribution of electron integrated path lengths.
:::

:::{image} ./_img/nist/e-stopping-power-ge-vs-estar.output.png
:width: 70%
:alt: Stopping power.
:::

Simulation of electrons in natural germanium with default settings (i.e. no
tuning of production cuts or step length).
```

### Electron interactions in liquid argon

Monoenergetic electrons are simulated in a liquid argon sphere of 15 cm radius.

```{subfigure} AA|BC
:subcaptions: above

:::{image} ./_img/nist/e-range-ar-vs-estar.output.png
:width: 50%
:alt: Electron integrated path lengths versus initial electron energy.
:::

:::{image} ./_img/nist/e-range-ar-distributions.output.png
:width: 70%
:alt: Distribution of electron integrated path lengths.
:::

:::{image} ./_img/nist/e-stopping-power-ar-vs-estar.output.png
:width: 70%
:alt: Stopping power.
:::

Simulation of electrons in natural germanium with default settings (i.e. no
tuning of production cuts or step length).
```

### Electron interactions in copper

Monoenergetic electrons are simulated in a copper sphere of 15 cm radius.

```{subfigure} AA|BC
:subcaptions: above

:::{image} ./_img/nist/e-range-cu-vs-estar.output.png
:width: 50%
:alt: Electron integrated path lengths versus initial electron energy.
:::

:::{image} ./_img/nist/e-range-cu-distributions.output.png
:width: 70%
:alt: Distribution of electron integrated path lengths.
:::

:::{image} ./_img/nist/e-stopping-power-cu-vs-estar.output.png
:width: 70%
:alt: Stopping power.
:::

Simulation of electrons in natural germanium with default settings (i.e. no
tuning of production cuts or step length).
```

:::{note}

Simulation and analysis scripts are available in
[`tests/nist`](https://github.com/legend-exp/remage/tree/main/tests/nist).

:::

# Comparison with NIST

In this section we compare the _remage_ simulation output with theoretical
predictions from the [NIST](https://www.nist.gov) database.

## Electron interactions in natural germanium

Monoenergetic electrons are simulated in a natural germanium sphere of 15 cm
radius. Multiple Coulomb scattering is disabled through the

```
/process/inactivate msc
```

command, since it prevents us to measure the true integrated particle path
length in the simulation (unless a very short step size limit is set).

We calculate stopping power and integrated path length of the simulated
electrons and compare them with data from the
[ESTAR database](https://physics.nist.gov/PhysRefData/Star/Text/ESTAR.html) (a
short description of the reported quantities is available
[here](https://physics.nist.gov/PhysRefData/Star/Text/method.html)).

:::{subfigure} AA|BC

:subcaptions: above

```{image} ./_img/nist/e-range-ge-vs-estar.output.png
:width: 50%
:alt: Electron integrated path lengths versus initial electron energy.
```

```{image} ./_img/nist/e-range-ge-distributions.output.png
:width: 70%
:alt: Distribution of electron integrated path lengths.
```

```{image} ./_img/nist/e-stopping-power-ge-vs-estar.output.png
:width: 70%
:alt: Stopping power.
```

Simulation of electrons in natural germanium with default settings (i.e. no
tuning of production cuts or step length).

:::

:::{note}

Simulation and analysis scripts are available in
[`tests/nist`](https://github.com/legend-exp/remage/tree/main/tests/nist).

:::

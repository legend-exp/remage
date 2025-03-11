# Double-beta decay physics

In this section we take a look at the primary spectrum produced by the
_bxdecay0_ package. The main documentation for this extension is available
[here](https://github.com/BxCppDev/bxdecay0). It aims to reproduce reasonable
primary particles to be used as generator for double beta decay physics.

## Primary electron properties

The main thing we can compare here are the properties of the primary electrons
produced. The _bxdecay0_ package actually does not even produce the neutrinos
emitted in e.g. $2\nu\beta\beta$.

```{subfigure} AB
:subcaptions: above

:::{image} ./_img/bxdecay0/double-beta-e-combined-primary-energy.output.png
:width: 95%
:alt: The combined energy of the two primary electrons.
:::

:::{image} ./_img/bxdecay0/double-beta-e-primary-opening-angle.output.png
:width: 95%
:alt: The opening angle between the two primary electrons.
:::

Different decay modes for the Ge76 decay.
```

Here $2\nu\beta\beta$ is the two neutrino double beta decay as predicted by the
Standard Model. $0\nu\beta\beta$ is the neutrinoless double beta decay channel
(assuming light majorana neutrino exchange) used for e.g. sensitivity estimates
by experiments like LEGEND. More exotic physics like the Majoron-emitting
neutrinoless double beta decay with SI=1 $0\nu\beta\beta\_M1$ shows a
distinctive continuous electron spectrum. Other exotic physics like the
$0\nu\beta\beta\_\lambda0$ is a decay mode driven by right-handed currents
according to the lambda mechanism and shows a distinctive angular correlation.

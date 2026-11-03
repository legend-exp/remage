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

## Majoron-emitting decay modes

The summed energy spectrum of the two electrons in Majoron-emitting neutrinoless
double beta decay depends on the spectral index $n$: the spectrum scales as
$(Q_{\beta\beta} - E)^n$, where $E$ is the summed electron energy. A larger
spectral index moves the spectrum to lower energies. For comparison, the
spectral index of $2\nu\beta\beta$ is $n = 5$.

```{figure} ./_img/bxdecay0/double-beta-majoron-combined-primary-energy.output.png
:width: 70%

Summed energy of the two primary electrons for the Majoron-emitting decay modes
of Ge76 with spectral index $n = 1, 2, 3, 7$, and for $2\nu\beta\beta$ ($n = 5$).
Spectra are normalized to unit area. The test checks that the mean summed
electron energy decreases with increasing spectral index.
```

## Decays to excited states

Ge76 can also decay to excited states of the daughter nucleus Se76. The
electrons then share at most $Q_{\beta\beta} - E_\text{level}$, where
$Q_{\beta\beta} = 2039$ keV and $E_\text{level}$ is the energy of the level. The
daughter nucleus de-excites by emitting gammas:

- $2^+_1$ (559 keV): one gamma at 559 keV
- $0^+_1$ (1122 keV): two gammas at 563 and 559 keV
- $2^+_2$ (1216 keV): one gamma at 1216 keV, or two gammas at 657 and 559 keV

```{figure} ./_img/bxdecay0/double-beta-excited-states-primary-energy.output.png
:width: 70%

Top: summed energy of the primary electrons for the decays of Ge76 to the ground
state and to the excited states of Se76. The dotted lines mark the endpoints
$Q_{\beta\beta} - E_\text{level}$. $0\nu\beta\beta\_\lambda$ to $2^+_1$ gives a
single line at the endpoint. Bottom: summed energy of the primary gammas, which
equals the energy of the excited level. The test checks the endpoints and the
summed gamma energies.
```

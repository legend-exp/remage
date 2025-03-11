# Gamma correlations

This section of the validation report shows the effect of angular correlations
after emission of multiple gammas after a decay. As an example: The background
rate of Co-60 depends on both gammas hitting one detector, so the angle between
the gammas matter.

## Expected correlations

See [presentation](https://indico.cern.ch/event/372884/contributions/1792361/)
for more background on the calculation.

The angular correlation functions take an approximate form of

```{math}
W(\theta) = 1 + a_2 \cos^2\theta + a_4 \cos^4\theta
```

For Co-60, the parameters $a_2 = 1/8$ and $a_4 = 1/24$ can be obtained from the
tables in [^gammacorr_table] by normalizing the 0th-coefficient to 1.

:::{todo}

- expectation for Tl-208

:::

## Simulated correlations

```{subfigure} AB
:subcaptions: above

:::{image} ./_img/gammacorr/gamma-angular-distribution-27-60.output.png
:width: 300px
:::

:::{image} ./_img/gammacorr/gamma-angular-distribution-81-208.output.png
:width: 300px
:::

Plots of the angle between two emitted gammas after a Co-60 and Tl-208 decay.
```

[^gammacorr_table]:
    H.W. Taylor, B. Singh, F.S. Prato, R. McPherson, A tabulation of gamma-gamma
    directional-correlation coefficients, Atomic Data and Nuclear Data Tables,
    Volume 9, Issue 1 (1971), Pages 1-83,
    [10.1016/S0092-640X(71)80040-2](<https://doi.org/10.1016/S0092-640X(71)80040-2>).

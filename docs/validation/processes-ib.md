# Internal Bremsstrahlung in Ar-39 decay

This section validates the simulation of Internal Bremsstrahlung (IB) radiation
in beta decay processes, specifically for Ar-39 which is a relevant background
source in liquid argon-based rare event searches.

## What is Internal Bremsstrahlung?

Internal Bremsstrahlung is a radiative correction to beta decay where a photon
is emitted along with the beta particle and neutrino. Unlike external
bremsstrahlung (which occurs when the beta particle interacts with matter after
emission), internal bremsstrahlung is emitted directly from the nucleus during
the decay process itself.

The IB photon energy spectrum is continuous, extending from zero up to the
Q-value of the decay. For Ar-39 (Q = 565 keV), the IB photons are typically soft
(low energy), with most photons below 0.5 MeV.

## Validation test

We simulate Ar-39 decays in a liquid argon environment with an HPGe detector and
compare simulations with and without IB enabled in Geant4. This allows us to
quantify the impact of IB on detector observables.

### IB gamma vertex energy spectrum

First, we extract the energy spectrum of photons emitted directly from Ar-39
decays (i.e., the IB photons themselves). This is done by selecting photons
whose parent track is an Ar-39 nucleus.

```{figure} ./_img/processes/innerbrem-gamma-vertex-energy.output.png
:width: 800px
*Energy spectrum of Internal Bremsstrahlung photons from Ar-39 decay. The
spectrum shows the characteristic soft photon distribution, with most photons
having energies below 0.5 MeV.*
```

&nbsp; &nbsp;

The spectrum shows the expected behavior: a continuous distribution dominated by
low-energy photons, with the rate decreasing at higher energies. This matches
theoretical predictions for IB emission.

### Detector signal comparison

Next, we compare the energy deposited in the HPGe detector for simulations with
and without IB enabled. This demonstrates the observable effect of IB on
detector response.

```{figure} ./_img/processes/innerbrem-signal.output.png
:width: 800px
*Comparison of energy deposited in the HPGe detector for Ar-39 decays with and
without Internal Bremsstrahlung. The spectra are rebinned to 20 keV bins for
clarity.*
```

&nbsp; &nbsp;

```{figure} ./_img/processes/innerbrem-difference.output.png
:width: 800px
*Difference in detector energy spectra (With IB - Without IB). The positive
values at low energies indicate additional events from IB photon interactions in
the detector.*
```

&nbsp; &nbsp;

The difference plot shows that including IB increases the number of low-energy
events in the detector. This is expected as the soft IB photons deposit
additional energy when they interact with the detector material. The effect is
most pronounced below ~100 keV, consistent with the IB photon energy spectrum.

These validation plots confirm that:

- IB photons are being generated with the correct energy distribution
- The detector response correctly accounts for IB contributions
- The magnitude of the effect is consistent with the $\sim10^{-3}$ branching
  ratio

For precision background modeling in low-background experiments, it is important
to enable IB in the simulation to accurately predict the detector response near
the analysis threshold.

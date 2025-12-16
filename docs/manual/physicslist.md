# Physics list

:::{todo}

- summary and peculiarities
- optional components and options (hadronic, leptonic, optical)
- production cuts and step limits
- custom WLS optical process
- custom Grabmayr gammacascades
- simulating radioactive decay chains

:::

## optional components

:::{todo}

- optional components and options (hadronic, leptonic, optical)

:::

### hadronic physics options

:::{todo}

- hadronic physics options

:::

Geant4 changed the default time threshold for radioactive decays in the 11.2
minor update to 1 year. _remage_ automatically reverts this change and sets the
threshold to the old default of Geant4 before 11.2, being `1.0e+27 ns`. This
threshold can be adjusted using the built-in command
`/process/had/rdm/thresholdForVeryLongDecayTime` after `/run/initialize`.

### Internal Bremsstrahlung

:::{todo}

- Internal Bremsstrahlung options

:::

Internal Bremsstrahlung is a radiative correction to beta decay where a photon
is emitted along with the beta particle and neutrino. Unlike external
bremsstrahlung (which occurs when the beta particle interacts with matter after
emission), internal bremsstrahlung is emitted directly from the nucleus during
the decay process itself.

The IB photon energy spectrum is continuous, extending from zero up to the
Q-value of the decay. For Ar-39 (Q = 565 keV), the IB photons are typically soft
(low energy), with most photons below 0.5 MeV. The spectrum is calculated based
on Hayen et al., Rev. Mod. Phys. 90, 015008 (2018).
https://journals.aps.org/rmp/abstract/10.1103/RevModPhys.90.015008 The
calculation is described in detail in Section V Radiative Correction.

The Inner Bremsstrahlung process is disabled by default in ReMage. To activate
Inner Bremsstrahlung process, please use the command
<project:../rmg-commands.md#rmgprocessesenableinnerbremsstrahlung> before
`/run/initialize`.

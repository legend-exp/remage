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
`/process/had/rdm/thresholdForVeryLongDecayTime` after `/run/initialize` .

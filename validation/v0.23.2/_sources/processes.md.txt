# Physics lists

This page lists the registered processes for a set of important particles for
various options for configuring the _remage_ physics list. The tables use the
Geant4 particle and process names.

:::{important}

- the `Transportation`, `StepLimiter` and `UserSpecialCut` are not shown
- most baryons and all (di-)mesons are not included in the list

:::

## Default (no extra options)

```{literalinclude} ./_img/processes/processes.default.output.txt
:class: dropdown
```

## With hadronic physics (shielding)

enabled optional physics list components:

```geant4
/RMG/Processes/HadronicPhysics Shielding
```

```{literalinclude} ./_img/processes/processes.hadr-shielding.output.txt
:class: dropdown
```

## With hadronic physics (shielding) with external gamma cascades

enabled optional physics list components:

```geant4
/RMG/Processes/HadronicPhysics Shielding
/RMG/Processes/UseGrabmayrsGammaCascades
```

```{literalinclude} ./_img/processes/processes.grabmayr.output.txt
:class: dropdown
```

## With inner bremsstrahlung enabled

enabled optional physics list components:

```geant4
/RMG/Processes/EnableInnerBremsstrahlung
```

```{literalinclude} ./_img/processes/processes.innerbrem.output.txt
:class: dropdown
```

## Optical physics enabled

enabled optional physics list components:

```geant4
/RMG/Processes/OpticalPhysics
```

```{literalinclude} ./_img/processes/processes.optical.output.txt
:class: dropdown
```

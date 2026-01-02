# Physics list

_remage_ uses a custom modular physics list built on top of Geant4’s standard
physics constructors. It combines:

- Configurable **electromagnetic physics** (standard, low-energy, Livermore,
  Penelope, polarized)
- Optional **optical physics** (scintillation, Cherenkov, absorption, Rayleigh,
  WLS)
- Configurable **hadronic physics lists**, including the Geant4 high-precision
  neutron (NeutronHP) models
- **Radioactive decay** with extended control over decay time thresholds
- Multiple **custom processes**:
  - Inner Bremsstrahlung for beta decay
  - Custom neutron capture with gamma cascades loaded from external files
  - Custom wavelength-shifting (WLS) optical process
- Production cuts are managed explicitly per-region, including a dedicated
  **SensitiveRegion**.

This physics list is designed and adjusted for:

- Low-energy nuclear and particle physics
- Underground or low-background experiments
- Precision gamma and beta spectroscopy
- Optical detector simulations
- Long-lived radioactive decay chains

## Electromagnetic physics

The electromagnetic physics is configurable via the
<project:../rmg-commands.md#rmgprocesseslowenergyemphysics> command.

Available options:

- `None` → `G4EmStandardPhysics`
- `Option{1,2,3,4}` → `G4EmStandardPhysics_option{1,2,3,4}`
- `Penelope` → `G4EmPenelopePhysics`
- `Livermore` → `G4EmLivermorePhysics` (**default**)
- `LivermorePolarized` → `G4EmLivermorePolarizedPhysics`

Some additional EM features from `G4EmExtraPhysics` are always enabled:

- Synchrotron radiation
- Gamma-nuclear interactions
- Muon-nuclear interactions
- e± nuclear interactions

## Optical physics

Optical physics is optional and can be enabled by
<project:../rmg-commands.md#rmgprocessesopticalphysics>.

This option enables the following physics processes: Refraction, reflection,
scintillation, Cherenkov radiation, optical bulk absorption, Rayleigh scattering
and wavelength shifting. The wavelength shifting (WLS) is either the default
implementation or a custom WLS process.

Global optical settings:

- Track scintillation secondaries first
- Enable scintillation by particle type
- Boundary process invokes sensitive detectors

### Custom optical wave length shifting (WLS) process

An optional custom wavelength-shifting process {cpp:class}`RMGOpWLSProcess` is
available and enabled by default. It can be disabled with
<project:../rmg-commands.md#rmgprocessesopticalphysicsmaxonewlsphoton>, if
necessary.

It wraps the standard `G4OpWLS` process, but ensures at most one secondary
photon is produced per WLS interaction. Most materials do not emit photons with
an efficiency of 100%, but a lower WLSE. In the default optical physics of
Geant4, setting such an efficiency enables the sampling of the number of emitted
photons from a distribution Poisson(WLSE). This will lead to the emission of
multiple photons in some cases.

We perceive this to be not the case in reality, but that the number of emitted
photons should rather be distributed along Bernoulli(WLSE). The custom WLS
process ensures this assumption holds true.

## Hadronic physics

Hadronic physics can be completely disabled or configured via predefined physics
lists.

If enabled, the following processes are included:

- Hadronic elastic scattering (`G4HadronElasticPhysicsHP`)
- Optional thermal neutron scattering (`G4ThermalNeutrons`)
- Hadronic inelastic physics (selectable)
- Stopping physics (`G4StoppingPhysics`)
- Ion physics (`G4IonPhysics`)

### Hadronic Physics Options

A hadronic physics option can be selected with
<project:../rmg-commands.md#rmgprocesseshadronicphysics>. Available options are

| Option         | Description                                                   |
| -------------- | ------------------------------------------------------------- |
| `None`         | No hadronic physics (**default**)                             |
| `QGSP_BIC_HP`  | Quark-Gluon String + Binary Cascade + HP neutrons [^QGSP_BIC] |
| `QGSP_BERT_HP` | QGSP with Bertini cascade + HP neutrons [^QGSP_BERT]          |
| `FTFP_BERT_HP` | Fritiof string model + Bertini + HP neutrons [^FTFP_BERT]     |
| `Shielding`    | Optimized shielding list with HP neutrons [^Shielding]        |

The Geant4 physics reference contains descriptions of the
[Fritiof](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsReferenceManual/html/hadronic/FTFmodel/FTFmodel.html)
and
[Bertini cascade](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsReferenceManual/html/hadronic/BertiniCascade/index.html)
models.

High-precision neutron cross sections (NeutronHP), as included in the individual
options.

An addition option,
<project:../rmg-commands.md#rmgprocessesenableneutronthermalscattering>, enables
thermal scattering for low-energy neutrons.

### Custom Grabmayr gamma cascades

When enabled with
<project:../rmg-commands.md#rmgprocessesusegrabmayrsgammacascades>, _remage_
replaces the standard neutron capture process (`nCapture`) with a custom one
that will generate gamma cascades for specific isotopes from files as provided
by P. Grabmayr _et al._ [^Grabmayr]. For all other isotopes, this process
exactly behaves the same.

This option is primarily intended for simulation scenarios that require
precision gamma spectroscopy following neutron capture.

:::{important}

The gamma cascade files are not shipped with _remage_ by default, but have to be
provided and registered by the user. For this, the command
<project:../rmg-commands.md#rmggrabmayrgammacascadessetgammacascadefile> can be
added to a macro file. See [^Grabmayr] for files to use for this.

:::

## Radioactive decays

Geant4 changed the default time threshold for radioactive decays in the 11.2
minor update to 1 year. _remage_ automatically reverts this change and sets the
threshold to the old default of Geant4 before 11.2, being `1.0e+27 ns`. This
threshold can be adjusted using the built-in command
`/process/had/rdm/thresholdForVeryLongDecayTime` after `/run/initialize`.

:::{seealso}

The primary particle generation for decay chains is handled by
{ref}`generators <manual-generators-decays>`.

:::

### Gamma angular correlations

When using Geant4 11.3 or higher, the simulation of angular correlations between
gammas emitted coincident with a radioative decay are enabled. This can be
controlled by
<project:../rmg-commands.md#rmgprocessesenablegammaangularcorrelation>.

### Internal Bremsstrahlung

Internal Bremsstrahlung is a radiative correction to beta decay where a photon
is emitted along with the beta particle and neutrino. Unlike external
bremsstrahlung (which occurs when the beta particle interacts with matter after
emission), internal bremsstrahlung is emitted directly from the nucleus during
the decay process itself.

The IB photon energy spectrum is continuous, extending from zero up to the
Q-value of the decay. For Ar-39 (Q = 565 keV), the IB photons are typically soft
(low energy), with most photons below 0.5 MeV. The spectrum is calculated based
on [^innerbremsstrahlung].

The Inner Bremsstrahlung process is disabled by default in _remage_. To activate
Inner Bremsstrahlung process, please use the command
<project:../rmg-commands.md#rmgprocessesenableinnerbremsstrahlung> before
`/run/initialize`.

## Production cuts and step limits

### Production cuts

Production cuts are specified in **length units** (as usual in Geant4), and
applied per particle type (gamma, electron, positron, proton).

Two regions are supported with separate production cuts, a default region (world
volume) and a sensitive region.

```geant4
/RMG/Processes/DefaultProductionCut {LENGTH} [mm]
/RMG/Processes/SensitiveProductionCut {LENGTH} [mm]
```

:::{note}

- The default cuts are tuned for low-energy applications (≈ 100 keV in Ge)
- The energy range for production cuts is explicitly set for low energy physics
  (200 eV to 100 GeV).
- All registered sensitive detectors will be automatically added to the
  sensitive region.

:::

### Step Limits

The `G4StepLimiterPhysics` is always added, but will not have an effect by
default. It allows volume-level step limits to be enforced if configured
elsewhere (e.g. in a GDML file).

:::{todo}

- document this somewhere.

:::

[^innerbremsstrahlung]:
    Hayen et al., in Rev. Mod. Phys. 90, 015008 (2018). doi:
    [10.1103/RevModPhys.90.015008](https://doi.org/10.1103/RevModPhys.90.015008).
    The calculation is described in detail in Section V (Radiative Correction).

[^QGSP_BIC]:
    see the "Hadronic Component" and "Related Physics Lists" sections in the
    [QGSP_BIC](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsListGuide/html/reference_PL/QGSP_BIC.html)
    physics list docs.

[^QGSP_BERT]:
    see the "Hadronic Component" and "Related Physics Lists" sections in the
    [QGSP_BERT](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsListGuide/html/reference_PL/QGSP_BERT.html)
    physics list docs.

[^FTFP_BERT]:
    see the "Hadronic Component" and "Related Physics Lists" sections in the
    [FTFP_BERT](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsListGuide/html/reference_PL/FTFP_BERT.html)
    physics list docs.

[^Shielding]:
    see the "Hadronic Component" and "Related Physics Lists" sections in the
    [Shielding](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsListGuide/html/reference_PL/Shielding.html)
    physics list docs.

[^Grabmayr]:
    See calculations of P. Grabmayr with MAURINA. The data files are available
    for inclusion with the simulation:

    - Cross sections and gamma cascades in 77Ge. publication:
      [10.1140/epja/s10050-024-01336-0](https://doi.org/10.1140/epja/s10050-024-01336-0)
      and [**data file**](https://zenodo.org/records/10222545).
    - Gamma cascades in gadolinium isotopes. publication:
      [10.1140/epjc/s10052-023-11602-y](https://doi.org/10.1140/epjc/s10052-023-11602-y)
      and [**data file**](https://zenodo.org/records/7458654).

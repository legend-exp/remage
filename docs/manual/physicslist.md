# Physics list

_remage_ uses a custom modular physics list built on top of Geant4’s standard
physics constructors. It mostly is based on the physics list implemented in
_MaGe_. It combines:

- Configurable **electromagnetic physics** (standard, low-energy, Livermore,
  Penelope, polarized)
- Optional **optical physics** (scintillation, Cherenkov, absorption, Rayleigh,
  WLS)
- Configurable **hadronic physics lists**, including high-precision neutron
  models
- **Radioactive decay** with extended control over decay time thresholds
- It contains Multiple **custom processes**:
  - Inner Bremsstrahlung for beta decay
  - Custom neutron capture with gamma cascades loaded from a Grabmayr file
  - Custom wavelength-shifting (WLS) optical process
- Production cuts are managed explicitly per-region, including a dedicated
  **SensitiveRegion**.

This physics list is designed and adjusted for:

- Low-energy nuclear and particle physics
- Underground or low-background experiments
- Precision gamma and beta spectroscopy
- Optical detector simulations
- Long-lived radioactive decay chains

## Electromagnetic (Leptonic & EM Physics)

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

## Optical Physics

Optical physics is optional and can be enabled by
<project:../rmg-commands.md#rmgprocessesopticalphysics>.

This option enables the following physics processes: Boundary interactions,
Scintillation, Cherenkov radiation, Optical absorption, Rayleigh scattering and
Wavelength shifting. The wavelength shifting is either the default
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
photon is produced per WLS interaction. For all our materials with a WLS
efficiency below one, this ensures that no extra photons are generated just by
setting this efficiency as in the stock process. This also matches our
expectation, as we do not expect that the emission of multiple photons in a WLS
material should be described by a Poisson distribution.

## Hadronic Physics

Hadronic physics can be completely disabled or configured via predefined physics
lists.

If enabled, the following processes are included:

- Hadronic elastic scattering (`G4HadronElasticPhysicsHP`)
- Optional thermal neutron scattering (`G4ThermalNeutrons`)
- Hadronic inelastic physics (selectable)
- Stopping physics (`G4StoppingPhysics`)
- Ion physics (`G4IonPhysics`)
- High-precision neutron cross sections (HP)

### Hadronic Physics Options

A hadronic physics option can be selected with
<project:../rmg-commands.md#rmgprocesseshadronicphysics>. Available options are

| Option         | Description                                         |
| -------------- | --------------------------------------------------- |
| `None`         | No hadronic physics (**default**)                   |
| `QGSP_BIC_HP`  | Quark-Gluon String + Binary Cascade + HP neutrons   |
| `QGSP_BERT_HP` | QGSP with Bertini cascade + HP neutrons             |
| `FTFP_BERT_HP` | Fritiof string model + Bertini + HP neutrons        |
| `Shielding`    | Optimized shielding list with HP neutrons (default) |

An addition option,
<project:../rmg-commands.md#rmgprocessesenableneutronthermalscattering>, enables
thermal scattering kernels for low-energy neutrons.

### Custom Grabmayr Gamma Cascades

When enabled with
<project:../rmg-commands.md#rmgprocessesusegrabmayrsgammacascades>, _remage_
replaces the standard neutron capture process (`nCapture`) with a custom one
that will generate gamma cascaded for specific isotopes from files as provided
by P. Grabmayr _et al._. For all other isotopes, this process exactly behaves
the same.

This option is primarily intended for scimulation sceneraios that require
precision gamma spectroscopy following neutron capture.

The gamma cascade files are not shipped with _remage_ by default, but have to be
provided and registered by the user. For this, the command
<project:../rmg-commands.md#rmggrabmayrgammacascadessetgammacascadefile> can be
added to a macro file.

## Radioactive decays

Geant4 changed the default time threshold for radioactive decays in the 11.2
minor update to 1 year. _remage_ automatically reverts this change and sets the
threshold to the old default of Geant4 before 11.2, being `1.0e+27 ns`. This
threshold can be adjusted using the built-in command
`/process/had/rdm/thresholdForVeryLongDecayTime` after `/run/initialize`.

:::{seealso}

Primary particle generation for decay chains is handled by
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
on [^1].

The Inner Bremsstrahlung process is disabled by default in ReMage. To activate
Inner Bremsstrahlung process, please use the command
<project:../rmg-commands.md#rmgprocessesenableinnerbremsstrahlung> before
`/run/initialize`.

## Production Cuts and Step Limits

### Production Cuts

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
elsewhere.

[^1]:
    Hayen et al., in Rev. Mod. Phys. 90, 015008 (2018). doi:
    [10.1103/RevModPhys.90.015008](https://doi.org/10.1103/RevModPhys.90.015008).
    The calculation is described in detail in Section V (Radiative Correction).

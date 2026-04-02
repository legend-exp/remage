# Muon physics validation

This page focuses on validating the physics of muons in remage, with a
particular emphasis on muon-induced neutron production and its relevance for
cosmogenic backgrounds in LEGEND.

## Introduction

Atmospheric muons are a relevant background source for underground
low-background experiments. For LEGEND at the Gran Sasso underground laboratory,
two classes of muon-induced backgrounds are important: prompt backgrounds from
direct energy deposition by muons and their secondaries, and delayed backgrounds
from radioactive isotopes produced in muon-induced showers.

For delayed backgrounds in LEGEND, one of the most relevant isotopes is
$^{77}$Ge, produced via neutron capture on $^{76}$Ge,
$^{76}\mathrm{Ge}(n,\gamma){}^{77}\mathrm{Ge}$ [^Ge77_2018] [^Ge77_2025]
[^Moritz_Thesis]. In particular, its isomeric state at 160 keV,
$^{77\mathrm{m}}\mathrm{Ge}$, is a concern for LEGEND-1000 because it likely
decays as a pure beta emitter, mimicking the neutrinoless double-beta decay
signal. To suppress it, delayed-tagging mechanisms have been developed to
identify the production of $^{77\mathrm{m}}\mathrm{Ge}$, though this requires a
robust understanding of muon-induced neutron production and transport in the
simulation.

To model muons, one first has to understand their initial energy distribution.
At sea level, the muon spectrum is commonly described by the Gaisser
parameterization [^Gaisser], while underground spectra are typically obtained by
propagating muons through rock with tools such as MUSIC/MUSUN [^MUSIC_MUSUN].
This leads to a broad underground spectrum, which motivates testing across wide
energy intervals rather than at a single representative value. At Gran Sasso,
muon energies range from tens of GeV to several TeV with an average around 270
GeV. In addition, muons traverse different materials before and within the
detector setup, in particular rock, water, and liquid argon. A meaningful
validation of muon physics in remage therefore has to cover both a broad energy
range and multiple materials.

This validation page focuses on five observables. For prompt backgrounds, the
key observable is the mean muon energy loss in matter, compared to PDG reference
tables [^pdg_energy_loss]. For delayed backgrounds, the key observable is the
muon-induced neutron yield, compared to both parameterizations and previous
simulations [^Wang] [^Kudryavtsev] [^Agafonova] [^Manukovsky]. To better
understand potential discrepancies, three additional observables for muon
showers are included: the relative contribution of different neutron production
channels, the development of the shower profile as a function of penetration
depth, and the production of radioactive isotopes in the shower.

Whenever possible, the observables are compared to experimental measurements and
to reference simulations from Geant4 and FLUKA. The present tests were designed
to be as close as possible to the setup in [^Manukovsky], which provides a
useful baseline for cross-version consistency checks.

:::{note}

The Geant4 version and G4NDL library used in [^Manukovsky] are outdated compared
to current production versions. In the future, it would be preferable to compare
against dedicated FLUKA simulations using an identical geometry and analysis
setup.

:::

## Energy loss of muons in matter

The energy loss of muons in matter consists of four main processes: ionization,
bremsstrahlung, pair production, and photonuclear interactions. Ionization is a
continuous process and dominates at lower energies, while the latter three are
stochastic and become increasingly relevant at higher energies. In water and
liquid argon, the transition between the two regimes is around 100 GeV.

In Geant4, ionization is implemented using three different models according to
the energy of the muon
[[Muon Ionisation]](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsReferenceManual/html/electromagnetic/muon_incident/muion.html),
while the other three processes are implemented using ultrarelativistic
approximation models
[[Muon Bremsstrahlung]](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsReferenceManual/html/electromagnetic/muon_incident/mubrem.html),
[[Muon Pair Production]](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsReferenceManual/html/electromagnetic/muon_incident/pair.html),
[[Muon Photonuclear Interaction]](https://geant4-userdoc.web.cern.ch/UsersGuides/PhysicsReferenceManual/html/electromagnetic/muon_incident/munu.html).

The mean energy loss from the individual processes is tabulated in the PDG
[^pdg_energy_loss], and the total energy loss is given by their sum. At high
energies, event-by-event energy loss fluctuations are large because stochastic
processes dominate. A full reconstruction of single-muon loss spectra (energy
straggling) and a clean decomposition of process-by-process contributions would
be interesting, but are complex to model and beyond the scope of this
validation.

The plots below show the mean energy loss of muons in water and liquid argon as
a function of the muon energy. The simulation estimates were generated by
shooting mono-energetic muons through a block of material, with the block length
chosen such that the muon loses about 5% of its initial energy. The simulated
mean values are then compared to the tabulated PDG expectation. Agreement should
be good over the full energy range.

```{image} ./_img/cosmogenics/muon/energy_loss_water_energy_range.output.png
:height: 300px
:alt: Energy loss of muons in water compared to DOI: 10.1006/adnd.2001.0861
```

```{image} ./_img/cosmogenics/muon/energy_loss_lar_energy_range.output.png
:height: 300px
:alt: Energy loss of muons in liquid argon compared to DOI: 10.1006/adnd.2001.0861
```

## Muon induced neutron yield

The muon-induced neutron yield is one of the most relevant observables for
delayed cosmogenic backgrounds, but it is also one of the most difficult to
model robustly.

Following [^Wang], the dominant neutron-production mechanisms are:

- Muon spallation via virtual-photon exchange. This channel is a major source of
  theoretical uncertainty.
- Muon elastic scattering on bound neutrons, summarized as $(\mu,\mu n)$.
- Photonuclear reactions from electromagnetic showers, $(\gamma,Xn)$.
- Secondary neutron multiplication following the primary channels, dominated by
  inelastic neutron reactions $(n,Xn)$.

Processes based on elastic scattering and photonuclear reactions are generally
better understood, while muon spallation and secondary neutron multiplication
dominate the modeling difficulty [^Wang]. Existing reference studies in FLUKA
and Geant4 [^Wang] [^Kudryavtsev] [^Manukovsky] provide important guidance, but
their numerical values depend on the chosen hadronic models and cross-section
data and can be associated with large uncertainties.

From the experimental side, data are scarce because neutron-yield measurements
require large detectors with good containment and high detection efficiency.
Water and liquid-scintillator experiments provide important constraints [^SK]
[^SNO], but direct measurements for liquid argon at relevant muon energies are
(to the knowledge of the author) not available.

The motivation for such parameterizations is discussed in [^Malgin], where the
observed scaling behavior is interpreted phenomenologically.

For this validation, we distinguish two parameterization classes:

- General parameterization: the global fit of $Y_n = c E_\mu^\alpha A^\beta$
  with $c = 4.4 \times 10^{-7}$, $\alpha = 0.78$, and $\beta = 0.95$ by
  [^Agafonova], extracted from experimental data across several materials.
- Material-specific parameterization: material-specific fits of
  $Y_n = c(A) E_\mu^{\alpha(A)}$, based on liquid scintillator FLUKA simulations
  by [^Wang] and [^Kudryavtsev] and general materials using Geant4 simulations
  by [^Manukovsky].

Comparing both helps separate broad phenomenology from model-dependent details.

For the present validation, computational constraints are significant. To reduce
computational cost, electrons, positrons, and gammas are only tracked down to
the lowest neutron separation energy in each material (slightly below 10 MeV in
the setups shown), and each configuration is limited to 20k events.

In the simulation, mono-energetic muons are shot through a cylinder of material
with a length of $2000 g/cm^2 \cdot \rho$ similar to [^Manukovsky]. The radius
was not reported in [^Manukovsky], but was chosen to be 1.25 m to ensure good
containment of the resulting showers. The neutron yield is then calculated as
the number of neutrons produced per muon per g/cm$^2$ after the burn-in depth
(see the shower dimensions section below).

The following tests are useful to validate that the simulations are in the right
range and to check for large discrepancies between simulation and expectation.

### Material dependence

Following the power-law framework introduced above, this subsection focuses on
the material dependence of the neutron yield. The comparison uses the
parameterization [^Agafonova] based on experimental trends together with the
material-specific Geant4-based study from [^Manukovsky] and the FLUKA
simulations by [^Wang] and [^Kudryavtsev] for water. A key feature from
[^Manukovsky] is that argon can deviate upwards from the generic trend of
[^Agafonova] because of its low neutron separation energy and high $(n,2n)$
cross-section.

The following plot shows the muon-induced neutron yield in different materials
at 100 GeV The agreement between the points and the material-specific
expectation should be good for all materials, the generalized parameterization
should give a rough estimate, though it is not expected to precisely match the
simulated values.

```{image} ./_img/cosmogenics/muon/neutron_yield_material_scan_100.0_Shielding_Livermore_30_8.045.output.png
:height: 300px
:alt: Muon induced neutron yield in different materials at 100 GeV. Compared to DOI: 10.1134/S106377881603011X
```

### Energy dependence

This subsection uses the same parameterizations as above but now emphasizes the
energy dependence of the neutron yield.

As mentioned above, in experiments, the initial muon energy is not known and can
vary up to several TeV. Instead, usually the neutron yield is associated with
the average muon energy for the corresponding experiment. This is related to the
overburden of the underground site resulting in Super-Kamiokande at about 2.7
km.w.e. having a lower mean energy of 259 GeV [^SK] compared to SNO+ with 364
GeV located at 6 km.w.e. [^SNO].

The following plots show the muon-induced neutron yield at 100 GeV and 280 GeV
(similar to [^Manukovsky]), the latter roughly corresponding to the mean muon
energy at Gran Sasso.

The first plot compares the remage estimated value for water with the
experimental values and parameterizations. The remage estimate should lie
approximately on the material-specific parameterization lines, which are in turn
consistent with measurements. Again, the generalized parameterization is
expected to deviate significantly from the simulated values.

```{image} ./_img/cosmogenics/muon/neutron_yield_energy_scan_water_Shielding_Livermore_30_8.045.output.png
:height: 300px
:alt: Muon induced neutron yield at different energies in water. Compared to DOI: 10.1134/S106377881603011X
```

The second plot compares the remage estimated value for liquid argon. Since only
parameterizations from [^Manukovsky] and [^Agafonova] are available, only these
are shown. The remage estimate should be consistent with the [^Manukovsky]
estimate, while lying above that of [^Agafonova].

```{image} ./_img/cosmogenics/muon/neutron_yield_energy_scan_lar_Shielding_Livermore_30_9.869.output.png
:height: 300px
:alt: Muon induced neutron yield at different energies in liquid argon. Compared to DOI: 10.1134/S106377881603011X
```

### Hadronic physics list

Geant4 offers different default hadronic physics lists using different models at
different energy ranges. Depending on the model chosen, there might be
differences in the resulting neutron yield. From previous estimates
[^Moritz_Thesis], differences in isotope production depending on the physics
lists are $<10\%$ with QGSP_BIC_HP expected to produce lower neutron yield.

In the following plot, the neutron yield for liquid argon at 100 GeV is compared
for different hadronic physics lists, with Shielding being the default used so
far. With the exposure simulated in this setup, no significant difference should
be visible.

```{image} ./_img/cosmogenics/muon/neutron_yield_had_physics_scan_100.0_lar_Livermore_30_9.869.output.png
:height: 300px
:alt: Muon induced neutron yield for different hadronic physics lists at 100 GeV in liquid argon. Compared to DOI: 10.1134/S106377881603011X
```

## Neutron production processes

Besides the total neutron yield, neutron production channels are an important
diagnostic for comparing different Geant4 versions and simulation frameworks. At
the relevant energies of 100 GeV and 280 GeV, the dominant channels are
photonuclear interactions $(\gamma,Xn)$ and neutron inelastic interactions
$(n,Xn)$, which generate secondary neutrons.

:::{note}

A caveat regarding inelastic neutron interactions is that in Geant4 the incoming
neutron track is terminated at these interactions and replaced by one or more
new outgoing neutron tracks with new track IDs. To avoid double counting, the
number of outgoing neutrons in these interactions is reduced by one. This
correction has already been applied in the results shown here.

:::

The plots below show the process-wise neutron contribution in liquid argon at
100 GeV and 280 GeV. The multiplicity of individual interactions is indicated by
color. At both energies, photonuclear and neutron inelastic interactions are
expected to contribute most of the total neutron yield, with neutron inelastic
interactions becoming more important at higher energies. Their multiplicity per
interaction is generally lower than for muon-, pion-, or proton-induced events.

```{image} ./_img/cosmogenics/muon/neutron_production_processes_100.0_lar_Shielding_Livermore_30_9.869.output.png
:height: 300px
:alt: Muon induced neutron production processes in liquid argon at 100 GeV.
```

```{image} ./_img/cosmogenics/muon/neutron_production_processes_280.0_lar_Shielding_Livermore_30_9.869.output.png
:height: 300px
:alt: Muon induced neutron production processes in liquid argon at 280 GeV.
```

## Muon shower dimensions

To compare muon shower physics across materials, it is useful to study the
development of the shower profile as a function of penetration depth.

First, it is important to estimate the "burn-in" depth at which the muon showers
have fully developed, i.e., the average particle flux and thus the average
energy deposition along the penetration depth stabilize. This is relevant
because, in deep-underground simulations, it is desirable to propagate muons
through the surrounding rock only as far as needed. The minimum simulation depth
is therefore set by the burn-in depth, after which the resulting particle flux
in the cavern no longer changes significantly. Since neutrons are the most
relevant particles for LEGEND, we also evaluate neutron production locations
along the depth.

The following plots were generated by summing all simulated muon events in both
the energy and neutron production profiles. The energy envelope is defined as
the radius from the beam axis that contains 99.5% of the total energy deposited
at each depth. The neutron production profile shows the locations of all
generated neutrons in the simulations as a function of depth and radius from the
beam axis.

The shower stabilization depth was estimated as follows: the individual energy
envelope points were plotted in a histogram according to their radius from the
beam axis. The mode and FWHM parameters were extracted to estimate the
stabilized shower width. Based on this, the first energy envelope point above
the threshold (mode - FWHM/2.335) was assumed to be the depth at which the
shower stabilizes. This metric is somewhat arbitrary and prone to statistical
fluctuations, but it is sufficiently robust for the purpose of this validation.

To the author's knowledge, there are no direct experimental measurements of this
burn-in depth or of the detailed neutron production profile. However, previous
estimates from stable versions of remage indicate that the burn-in depth at 100
GeV should be roughly 3-4 m in water, 1-2 m in liquid argon, and 1 m in rock.
The estimated burn-in depth should be consistent across both the energy
containment envelope and the neutron production profile.

A second point of interest is the width of the average shower after
stabilization. This parameter, like the burn-in depth, is material-dependent and
can signal changes in the underlying physics models if it shifts suddenly
between Geant4/remage versions. The stabilized width should be roughly 40 cm in
water, 30 cm in liquid argon, and 20 cm in rock.

```{image} ./_img/cosmogenics/muon/shower_dimensions_100.0_lar_Shielding_Livermore_30_9.869.output.png
:height: 300px
:alt: Muon shower profile in liquid argon at 100 GeV.
```

```{image} ./_img/cosmogenics/muon/shower_dimensions_100.0_rock_Shielding_Livermore_30_8.045.output.png
:height: 300px
:alt: Muon shower profile in rock at 100 GeV.
```

```{image} ./_img/cosmogenics/muon/shower_dimensions_100.0_water_Shielding_Livermore_30_8.045.output.png
:height: 300px
:alt: Muon shower profile in water at 100 GeV.
```

:::{note}

The burn-in depth also depends on energy. Considering the wide muon energy range
up to several TeV, it is recommended to use burn-in depths of up to ~3 m in
rock. More thorough estimates should be added here in the future.

:::

## Isotope production

:::{note}

This part is still in development. Ideally, these results, especially isotopes
produced via spallation, should be compared to FLUKA. Isotopes produced via
neutron capture need additional treatment as laid out in the neutron physics
validation section.

:::

## References

[^pdg_energy_loss]:
    DONALD E. GROOM, NIKOLAI V. MOKHOV, SERGEI I. STRIGANOV, MUON STOPPING POWER
    AND RANGE TABLES 10 MeV–100 TeV, Atomic Data and Nuclear Data Tables, Volume
    78, Issue 2, 2001, Pages 183-356,
    [10.1006/adnd.2001.0861](https://doi.org/10.1006/adnd.2001.0861).

[^Wang]:
    Y.-F. Wang, V. Balic, G. Gratta, A. Fassò, S. Roesler, and A. Ferrari,
    Predicting neutron production from cosmic-ray muons, Physical Review D 64,
    013012 (2001),
    [10.1103/physrevd.64.013012](https://doi.org/10.1103/physrevd.64.013012).

[^Kudryavtsev]:
    V. A. Kudryavtsev, N. J. C. Spooner, and J. E. McMillan, Simulations of
    muon-induced neutron flux at large depths underground, Nuclear Instruments
    and Methods in Physics Research Section A: Accelerators, Spectrometers,
    Detectors and Associated Equipment, Volume 505, Issues 3, 2003, Pages
    688-698,
    [10.1016/S0168-9002(03)00983-5](<https://doi.org/10.1016/S0168-9002(03)00983-5>).

[^Agafonova]:
    N. Yu. Agafonova and A. S. Malgin, Universal formula for the muon-induced
    neutron yield, Phys. Rev. D 87, 113013 – Published 27 June, 2013,
    [10.1103/physrevd.87.113013](https://doi.org/10.1103/physrevd.87.113013).

[^Malgin]:
    A. S. Malgin, Phenomenological analysis of the muon-induced neutron yield,
    Phys. Rev. D 88, 073003 – Published 11 October, 2013,
    [10.1103/physrevd.88.073003](https://doi.org/10.1103/physrevd.88.073003).

[^SK]:
    Super-Kamiokande Collaboration, Measurement of the cosmogenic neutron yield
    in Super-Kamiokande with gadolinium loaded water,
    [10.1103/PhysRevD.107.092009](https://doi.org/10.1103/PhysRevD.107.092009).

[^SNO]:
    SNO+ Collaboration, Cosmogenic Neutron Production in Water at SNO+,
    [10.1103/vs3y-sbb2](https://doi.org/10.1103/vs3y-sbb2).

[^Manukovsky]:
    K. V. Manukovsky, O. G. Ryazhskaya, N. M. Sobolevsky & A. V. Yudin, Neutron
    production by cosmic-ray muons in various materials, Phys. Atom. Nuclei 79,
    631–640 (2016),
    [10.1134/S106377881603011X](https://doi.org/10.1134/S106377881603011X).

[^Ge77_2018]:
    Virtual depth by active background suppression: revisiting the cosmic muon
    induced background of GERDA Phase II, Christoph Wiesinger, Luciano Pandola,
    and Stefan Schoenert, Eur. Phys. J. C 78, 597 (2018),
    [10.1140/epjc/s10052-018-6079-3](https://doi.org/10.1140/epjc/s10052-018-6079-3).

[^Ge77_2025]:
    Search for the in-situ production of Ge in the GERDA neutrinoless
    double-beta decay experiment, GERDA Collaboration, Eur. Phys. J. C 85, 809
    (2025),
    [10.1140/epjc/s10052-025-14445-x](https://doi.org/10.1140/epjc/s10052-025-14445-x).

[^Gaisser]:
    T. K. Gaisser, Cosmic Rays and Particle Physics, Cambridge University Press,
    [10.1017/CBO9781139192194](https://doi.org/10.1017/CBO9781139192194).

[^MUSIC_MUSUN]:
    Muon simulation codes MUSIC and MUSUN for underground physics, V. A.
    Kudryavtsev, Computer Physics Communications, Volume 180, Issue 3, 2009,
    Pages 339-346,
    [10.1016/j.cpc.2008.10.013](https://doi.org/10.1016/j.cpc.2008.10.013).

[^Moritz_Thesis]:
    Moritz Neuberger, None of the above - A comprehensive study of the
    muon-induced backgrounds in the LEGEND experiment.

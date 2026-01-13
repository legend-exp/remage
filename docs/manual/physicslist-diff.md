The default physics list is `FTFP_BERT` [^default_physlist], essentially with
electromagnetic option `_EM0`.

legend:

- ➖ = don't care / optional components that make sense
- ✅ = `RMGPhysics` matches with G4 reference list
- ⬆️ = `RMGPhysics` differs from the G4 reference list, but it is more suitable
  for use case
- ❓ = needs investigation
- ⚠️ = **(potential) problem**

### electromagnetic physics

⬆️ the reference list uses EMStandard (`_EM0`), whereas `RMGPhyiscs` uses
Livermore (`_LIV`) physics. I guess this is suitable for our use. `QGSP_BIC_HP`
uses `EMZ`

✅ Both enable `G4EmExtraPhysics` and use mostly defaults.

⬆️/❓ for `G4EmExtraPhysics`, `RMGPhyiscs` also enables Synch for e±, but _not_
for mu±, p±, pi± or generic ions.

### hadronic physics

`FTFP_BERT` (obviously) uses the `FTFP_BERT` hadronic physics by default.

⚠️ **`RMGPhyiscs` does not use any hadronic physics at all by default!** This
does include all physics listed in this section.

✅ for the hadronic physics, `FTFP_BERT_HP` and `RMGPhysics` configured which
hadronic option `FTFP_BERT_HP` use the same physics constructor:

- `G4StoppingPhysics` (anti-proton, anti-Sigma+, mu-, pi-, K-, Sigma-, Xi-,
  Omega-, anti-neutron, anti-Lambda, anti-Sigma0 and anti-Xi0)
- `G4IonPhysics` (d, t, alpha, ions)
- `G4HadronElasticPhysicsHP`
- `G4HadronPhysics{OPTION}_HP`

- ➖ optional in RMG if hadronic enabled: `G4ThermalNeutrons`
  - this would probably correspond to a `_HPT` physics list
- ⬆️ optional in RMG: Grabmayr gamma cascades

### hadronic physics - `Shielding`

⚠️ when using the Shielding option in remage for hadronic physics, `RMGPhysics`
works differently than the `Shielding` physics list!

⚠️ The `Shielding` physics list does _not_ use `G4IonPhysics` as in
`RMGPhysics`: It rather uses `G4IonElasticPhysics` and
`G4LightIonQMDPhysics/G4IonQMDPhysics`.

⚠️ The `Shielding` physics list also enables `SetProduceFissionFragments`, which
`RMGPhysics` does not.

### hadronic physics - `QGSP_BERT_HP`/`QGSP_BIC_HP`

⚠️ The `QGSP_BERT_HP` physics list does _not only_ use `G4IonPhysics` as in
`RMGPhysics`: It _additionally_ uses `G4IonElasticPhysics`.

### cut values

- `FTFP_BERT` sets 0.7 mm for all applicable particles
- ⬆️ `RMGPhysics` sets 0.1 mm, better suited for Germanium experiments
- let's look at the `_HP` option that remage uses for hadron physics:
  - ⚠️ `FTFP_BERT_HP`/`Shielding`/`QGSP_BERT_HP`/`QGSP_BIC_HP` set 0.7 mm for
    all applicable particles **_except_ for protons where it sets 0 mm "for
    producing low energy recoil nucleus"**
  - explanation for the zero cut: [^proton0cut]

### optical physics

This is never enabled by default, but comparing our manual implementation with
`G4OpticalPhysics` physlist constructor.

- ✅ mostly matches
- ➖ OpWLS2 & OpMieHG enabled by default in `G4OpticalPhysics` and not by us,
  but we do not (currently) use them
- ❓ `G4OpticalPhysics` adds a saturation process to scintillation
  ```
  G4EmSaturation* emSaturation = G4LossTableManager::Instance()->EmSaturation();
  scint->AddSaturation(emSaturation);
  ```
  this disables handling of the Birk's saturation, if enabled for a material,
  otherwise no-op [^birks].
- ⚠️ probably an oversight: missing explicit process index setting for boundary
  proc. **I need to check if this has any effect. it might be that our process
  order was broken all the time!**

### other

- ✅ `G4DecayPhysics` and `G4RadioactiveDecayPhysics` are added by
  `FTFP_BERT_HP`/`Shielding`/`QGSP_BERT_HP`/`QGSP_BIC_HP` as well as
  `RMGPhysics`
  - the non-`_HP` physics lists do _not_ add `G4RadioactiveDecayPhysics` for
    some reason? This is documented.

- ➖ `RMGPhyiscs` adds `G4StepLimiterPhysics` by default, but this is a no-op
  without further config.
- ⬆️ (optional) `RMGPhyiscs` add inner bremsstrahlung

## Next steps and final notes

- I am currently investigating the optical differences.
- I might switch to `G4OpticalPhysics` at some point.
- @MoritzNeuberger: can you have a look at the `Shielding` differences above?
- should we adopt the proton cut of 0mm?
- is running without any hadronic physics fine?

## About the physics list factory

- the factory is modular, i.e. you can add other physics constructors by
  RegisterPhysics/ReplacePhysics from the compound phys list name, e.g.
  `FTFP_BERT_HPT_LIV+G4OpticalPhysics+G4StepLimiterPhysics` would be _roughly_
  equivalent to our physics list with all "built-in" options enabled.
- However, there is no way to call `RemovePhysics` from the physics list name.
  - we cannot achieve our current default physics list w/o hadronic physics
    without custom C++ code
- option: use the `RegisterPhysics` facility from our own class to make our code
  shorter.
  - this is also not so easy, as we still would need to honour our own optional
    components and options.

[^default_physlist]:
    https://geant4.web.cern.ch/documentation/dev/plg_html/PhysicsListGuide/physicslistguide.html
    „Since the Geant4 release 10.0 the default Physics List becomes FTFP_BERT“

[^birks]:
    - note 1: we have added some logic to read Birk's constant from GDML, so we
      either should remove that option or enable the EmSaturation?
    - note 1a: however, we do not set BIRKSCONSTANT in GDML in any of our
      geometries 😅...
    - note 1b: maybe we should add it optionally?
    - note 2 (to myself): this is an interesting interface to potentially
      implement energy/particle-dependant non-Birk's quenching in scintillators?

[^proton0cut]:
    https://geant4.web.cern.ch/documentation/pipelines/master/bfad_html/ForApplicationDevelopers/TrackingAndPhysics/physicsProcess.html#note-on-the-time-threshold-for-radioactive-decay-of-ions
    „Moreover, in these cases, the results are also depending on the value of
    the range threshold for proton (because target nuclei which receive an
    elastic recoil above a certain kinetic energy become tracks, and therefore
    can have radioactive decays).“

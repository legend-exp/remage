#include "RMGPhysics.hh"

#include "G4ProcessManager.hh"
#include "G4RegionStore.hh"
#include "G4HadronicProcessStore.hh"
#include "G4StepLimiter.hh"
#include "G4NuclearLevelData.hh"
#include "G4DeexPrecoParameters.hh"
#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"
#include "G4RunManagerKernel.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4EmStandardPhysics_option2.hh"
#include "G4EmStandardPhysics_option3.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4EmLivermorePolarizedPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4HadronicParameters.hh"
#include "G4IonTable.hh"
#include "G4OpticalParameters.hh"
#include "G4Scintillation.hh"
#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4OpWLS.hh"
#include "G4Cerenkov.hh"

#include "RMGLog.hh"
#include "RMGTools.hh"

namespace u = CLHEP;

RMGPhysics::RMGPhysics() :
  G4VModularPhysicsList() {

  G4VUserPhysicsList::defaultCutValue = 0.1*u::mm;
  this->SetPhysicsRealm(RMGPhysics::kDoubleBetaDecay);

  G4VModularPhysicsList::verboseLevel = RMGLog::GetLogLevelScreen() <= RMGLog::debug ? 1 : 0;

  this->DefineCommands();
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetUseGammaAngCorr(bool b) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetCorrelatedGamma(b);
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetGammaTwoJMAX(int max_two_j) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetCorrelatedGamma(true);
  pars->SetTwoJMAX(max_two_j);
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetStoreICLevelData(bool store) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetStoreICLevelData(store);
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::ConstructParticle() {

  RMGLog::Out(RMGLog::detail, "Constructing particles");

  G4BosonConstructor boson_const;
  G4LeptonConstructor lepton_const;
  G4MesonConstructor meson_const;
  G4BaryonConstructor baryon_const;
  G4IonConstructor ion_const;
  G4ShortLivedConstructor short_lived_const;

  boson_const.ConstructParticle();
  lepton_const.ConstructParticle();
  meson_const.ConstructParticle();
  baryon_const.ConstructParticle();
  ion_const.ConstructParticle();
  short_lived_const.ConstructParticle();

  return;
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::ConstructProcess() {

  G4VUserPhysicsList::AddTransportation();

  // EM Physics
  G4VPhysicsConstructor* em_constructor = nullptr;
  RMGLog::Out(RMGLog::detail, "Adding electromagnetic physics");
  if (fUseLowEnergyEM) {
    switch (fLowEnergyEMOption) {
      // from https://geant4.web.cern.ch/node/1731
      case RMGPhysics::LowEnergyEMOption::kOption1 :
        em_constructor = new G4EmStandardPhysics_option1(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 1");
        break;
      case RMGPhysics::LowEnergyEMOption::kOption2 :
        em_constructor = new G4EmStandardPhysics_option2(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 2");
        break;
      case RMGPhysics::LowEnergyEMOption::kOption3 :
        em_constructor = new G4EmStandardPhysics_option3(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 3");
        break;
      case RMGPhysics::LowEnergyEMOption::kOption4:
        em_constructor = new G4EmStandardPhysics_option4(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 4");
        break;
      case RMGPhysics::LowEnergyEMOption::kPenelope :
        em_constructor = new G4EmPenelopePhysics(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using Penelope Physics");
        break;
      case RMGPhysics::LowEnergyEMOption::kLivermorePolarized :
        em_constructor = new G4EmLivermorePolarizedPhysics(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using Livermore-Polarized Physics");
        break;
      case RMGPhysics::LowEnergyEMOption::kLivermore :
        RMGLog::Out(RMGLog::detail, "Using Livermore/LowEnergy electromagnetic physics");
        em_constructor = new G4EmLivermorePhysics(G4VModularPhysicsList::verboseLevel);
        break;
    }
  }
  else {
    RMGLog::Out(RMGLog::detail, "Using Standard electromagnetic physics");
    em_constructor = new G4EmStandardPhysics(G4VModularPhysicsList::verboseLevel);
  }

  em_constructor->ConstructProcess();

  // Includes synchrotron radiation, gamma-nuclear, muon-nuclear and
  // e+/e- nuclear interactions
  RMGLog::Out(RMGLog::detail, "Adding extra electromagnetic physics");
  auto em_extra_physics = new G4EmExtraPhysics(G4VModularPhysicsList::verboseLevel);
  em_extra_physics->Synch("on");
  em_extra_physics->GammaNuclear("on");
  em_extra_physics->MuonNuclear("on");
  em_extra_physics->ConstructProcess();

  if (fConstructOptical) this->ConstructOptical();
  else RMGLog::Out(RMGLog::detail, "Processes for optical photons are inactivated");

  // Add decays
  RMGLog::Out(RMGLog::detail, "Adding radioactive decay physics");
  auto decay_physics = new G4DecayPhysics(G4VModularPhysicsList::verboseLevel);
  decay_physics->ConstructProcess();
  auto rad_decay_physics = new G4RadioactiveDecayPhysics(G4VModularPhysicsList::verboseLevel);
  rad_decay_physics->ConstructProcess();
  const auto the_ion_table = G4ParticleTable::GetParticleTable()->GetIonTable();
  RMGLog::Out(RMGLog::detail, "Entries in ion table ", the_ion_table->Entries());
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::ConstructOptical() {

  RMGLog::Out(RMGLog::detail, "Adding optical physics");

  G4OpticalParameters* op_par = G4OpticalParameters::Instance();
  op_par->SetScintTrackSecondariesFirst(true);
  op_par->SetScintByParticleType(true);
  op_par->SetBoundaryInvokeSD(true);

  // scintillation process
  auto scint_proc = new G4Scintillation("Scintillation");
  scint_proc->SetTrackSecondariesFirst(true);
  scint_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  // optical processes
  auto absorption_proc     = new G4OpAbsorption();
  auto boundary_proc       = new G4OpBoundaryProcess();
  auto rayleigh_scatt_proc = new G4OpRayleigh();
  auto wls_proc            = new G4OpWLS();
  auto cerenkov_proc       = new G4Cerenkov();

  absorption_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  boundary_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  wls_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  GetParticleIterator()->reset();
  while ((*GetParticleIterator())()) {
    auto particle = GetParticleIterator()->value();
    auto proc_manager = particle->GetProcessManager();
    auto particle_name = particle->GetParticleName();

    if (scint_proc->IsApplicable(*particle)) {
      proc_manager->AddProcess(scint_proc);
      proc_manager->SetProcessOrderingToLast(scint_proc, G4ProcessVectorDoItIndex::idxAtRest);
      proc_manager->SetProcessOrderingToLast(scint_proc, G4ProcessVectorDoItIndex::idxPostStep);
    }

    if (cerenkov_proc->IsApplicable(*particle)) {
      proc_manager->AddProcess(cerenkov_proc);
      proc_manager->SetProcessOrdering(cerenkov_proc, G4ProcessVectorDoItIndex::idxPostStep);
    }

    if (particle_name == "opticalphoton") {
      proc_manager->AddDiscreteProcess(absorption_proc);
      proc_manager->AddDiscreteProcess(boundary_proc);
      proc_manager->AddDiscreteProcess(rayleigh_scatt_proc);
      proc_manager->AddDiscreteProcess(wls_proc);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

// TODO: define multiple sensitive regions? look in detector construction too
void RMGPhysics::SetCuts() {

  RMGLog::Out(RMGLog::debug, "Setting particle cut values");

  G4HadronicProcessStore::Instance()->SetVerbose(G4VModularPhysicsList::verboseLevel);
  // special for low energy physics
  G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(250*u::eV, 100.*u::GeV);

  this->SetCutValue(fStepCuts.gamma, "gamma");
  this->SetCutValue(fStepCuts.electron, "e-");
  this->SetCutValue(fStepCuts.positron, "e+");
  this->SetCutValue(fStepCuts.proton, "proton");
  this->SetCutValue(fStepCuts.alpha, "alpha");
  this->SetCutValue(fStepCuts.generic_ion, "GenericIon");

  if (G4RegionStore::GetInstance()) {
    if (G4RegionStore::GetInstance()->size() > 1) {
      // Set different cuts for the sensitive region
      auto region = G4RegionStore::GetInstance()->GetRegion("SensitiveRegion", false);
      if (region) {
        RMGLog::Out(RMGLog::detail, "Register cuts for SensitiveRegion ");
        auto cuts = region->GetProductionCuts();
        if (!cuts) cuts = new G4ProductionCuts;
        cuts->SetProductionCut(fStepCutsSensitive.gamma, "gamma");
        cuts->SetProductionCut(fStepCutsSensitive.electron, "e-");
        cuts->SetProductionCut(fStepCutsSensitive.positron, "e+");
        cuts->SetProductionCut(fStepCutsSensitive.proton, "proton");
        cuts->SetProductionCut(fStepCutsSensitive.alpha, "alpha");
        cuts->SetProductionCut(fStepCutsSensitive.generic_ion, "GenericIon");
        region->SetProductionCuts(cuts);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetPhysicsRealm(PhysicsRealm realm) {
  switch (realm) {
    case RMGPhysics::PhysicsRealm::kDoubleBetaDecay :
      RMGLog::Out(RMGLog::summary, "Realm set to DoubleBetaDecay");
      // The default values for the energy thresholds are tuned to 100 keV
      // in natural germanium (i.e., the BBdecay realm)
      fStepCuts = StepCutStore(G4VUserPhysicsList::defaultCutValue);
      fStepCuts.gamma    = 0.1*u::mm;
      fStepCuts.electron = 0.1*u::mm;
      fStepCuts.positron = 0.1*u::mm;

      fStepCutsSensitive = StepCutStore(G4VUserPhysicsList::defaultCutValue);
      fStepCutsSensitive.gamma    = 0.1*u::mm;
      fStepCutsSensitive.electron = 0.1*u::mm;
      fStepCutsSensitive.positron = 0.1*u::mm;
      break;

    case RMGPhysics::PhysicsRealm::kDarkMatter :
      RMGLog::Out(RMGLog::summary, "Realm set to DarkMatter");
      // These values are tuned to ~1 keV for gamma, e+, e- in
      // natural germanium.
      fStepCuts = StepCutStore(G4VUserPhysicsList::defaultCutValue);
      fStepCuts.gamma    = 5*u::um;
      fStepCuts.electron = 0.5*u::um;
      fStepCuts.positron = 0.5*u::um;

      fStepCutsSensitive = StepCutStore(G4VUserPhysicsList::defaultCutValue);
      fStepCutsSensitive.gamma    = 5*u::um;
      fStepCutsSensitive.electron = 0.5*u::um;
      fStepCutsSensitive.positron = 0.5*u::um;
      break;

    case RMGPhysics::PhysicsRealm::kCosmicRays :
      RMGLog::Out(RMGLog::summary, "Realm set to CosmicRays (cut-per-region)");
      fStepCuts = StepCutStore(G4VUserPhysicsList::defaultCutValue);
      fStepCuts.gamma       = 5*u::cm;
      fStepCuts.electron    = 1*u::cm;
      fStepCuts.positron    = 1*u::cm;
      fStepCuts.proton      = 5*u::mm;
      fStepCuts.alpha       = 5*u::mm;
      fStepCuts.generic_ion = 5*u::mm;

      fStepCutsSensitive = StepCutStore(G4VUserPhysicsList::defaultCutValue);
      fStepCutsSensitive.gamma    = 30*u::mm;
      fStepCutsSensitive.electron = 40*u::um;
      fStepCutsSensitive.positron = 40*u::um;
      break;

    case RMGPhysics::PhysicsRealm::kLArScintillation :
      RMGLog::Out(RMGLog::warning, "LAr scintillation realm unimplemented");
  }

  this->SetCuts();
  fPhysicsRealm = realm;
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetLowEnergyEMOptionString(std::string option) {
  try { fLowEnergyEMOption = RMGTools::ToEnum<RMGPhysics::LowEnergyEMOption>(option, "low energy EM option"); }
  catch (const std::bad_cast&) { return; }
}

void RMGPhysics::SetPhysicsRealmString(std::string realm) {
  try { this->SetPhysicsRealm(RMGTools::ToEnum<RMGPhysics::PhysicsRealm>(realm, "physics realm")); }
  catch (const std::bad_cast&) { return; }
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Processes/",
      "Commands for controlling physics processes");

  fMessenger->DeclareMethod("Realm", &RMGPhysics::SetPhysicsRealmString)
    .SetGuidance("Set simulation realm (cut values for particles in (sensitive) detector")
    .SetParameterName("realm", false)
    .SetCandidates(RMGTools::GetCandidates<RMGPhysics::PhysicsRealm>())
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareProperty("OpticalPhysics", fConstructOptical)
    .SetGuidance("Add optical processes to the physics list")
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("LowEnergyEMPhysics", &RMGPhysics::SetLowEnergyEMOptionString)
    .SetGuidance("Add low energy electromagnetic processes to the physics list")
    .SetCandidates(RMGTools::GetCandidates<RMGPhysics::LowEnergyEMOption>())
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("EnableGammaAngularCorrelation", &RMGPhysics::SetUseGammaAngCorr)
    .SetGuidance("")
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("GammaTwoJMAX", &RMGPhysics::SetGammaTwoJMAX)
    .SetGuidance("")
    .SetParameterName("x", false)
    .SetRange("x > 0")
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("StoreICLevelData", &RMGPhysics::SetStoreICLevelData)
    .SetGuidance("")
    .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: shiftwidth=2 tabstop=2 expandtab

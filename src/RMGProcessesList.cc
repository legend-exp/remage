#include "RMGProcessesList.hh"

#include "G4ProcessManager.hh"
#include "G4RegionStore.hh"
#include "G4HadronicProcessStore.hh"
#include "G4StepLimiter.hh"
#include "G4ParallelWorldScoringProcess.hh"
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
#include "G4RadioactiveDecayBase.hh"
#include "G4HadronicParameters.hh"
#include "G4IonTable.hh"
#include "G4Scintillation.hh"
#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4OpWLS.hh"
#include "G4Cerenkov.hh"

#include "RMGProcessesMessenger.hh"
#include "RMGLog.hh"
#include "RMGTools.hh"

namespace u = CLHEP;

RMGProcessesList::RMGProcessesList() :
  G4VModularPhysicsList() {

  G4VUserPhysicsList::defaultCutValue = 0.1*u::mm;
  this->SetPhysicsRealm(RMGProcessesList::kDoubleBetaDecay);

  G4VModularPhysicsList::verboseLevel = 0;
  this->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  fUseLowEnergyEM     = true;
  fLowEnergyEMOption  = RMGProcessesList::LowEnergyEMOption::kLivermore;
  fConstructOptical   = false;

  this->DefineCommands();
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::SetUseGammaAngCorr(G4bool b) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetCorrelatedGamma(b);
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::SetGammaTwoJMAX(G4int max_two_j) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetCorrelatedGamma(true);
  pars->SetTwoJMAX(max_two_j);
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::SetStoreICLevelData(G4bool store) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetStoreICLevelData(store);
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::ConstructParticle() {

  RMGLog::Out(RMGLog::detail, "Constructing particles");

  G4BosonConstructor boson_const;
  boson_const.ConstructParticle();

  G4LeptonConstructor lepton_const;
  lepton_const.ConstructParticle();

  G4MesonConstructor meson_const;
  meson_const.ConstructParticle();

  G4BaryonConstructor baryon_const;
  baryon_const.ConstructParticle();

  G4IonConstructor ion_const;
  ion_const.ConstructParticle();

  G4ShortLivedConstructor short_lived_const;
  short_lived_const.ConstructParticle();

  return;
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::ConstructProcess() {

  this->AddTransportation();

  // parallel worlds must be added after G4Transportation
  if (G4RunManagerKernel::GetRunManagerKernel()->GetNumberOfParallelWorld() > 0) {
    this->AddParallelWorldScoring();
  }

  // EM Physics
  G4VPhysicsConstructor* em_constructor = nullptr;
  RMGLog::Out(RMGLog::detail, "Adding electromagnetic physics");
  if (fUseLowEnergyEM) {
    switch (fLowEnergyEMOption) {
      // from https://geant4.web.cern.ch/node/1731
      case RMGProcessesList::LowEnergyEMOption::kOption1 :
        em_constructor = new G4EmStandardPhysics_option1(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 1");
        break;
      case RMGProcessesList::LowEnergyEMOption::kOption2 :
        em_constructor = new G4EmStandardPhysics_option2(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 2");
        break;
      case RMGProcessesList::LowEnergyEMOption::kOption3 :
        em_constructor = new G4EmStandardPhysics_option3(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 3");
        break;
      case RMGProcessesList::LowEnergyEMOption::kOption4:
        em_constructor = new G4EmStandardPhysics_option4(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 4");
        break;
      case RMGProcessesList::LowEnergyEMOption::kPenelope :
        em_constructor = new G4EmPenelopePhysics(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using Penelope Physics");
        break;
      case RMGProcessesList::LowEnergyEMOption::kLivermorePolarized :
        em_constructor = new G4EmLivermorePolarizedPhysics(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using Livermore-Polarized Physics");
        break;
      case RMGProcessesList::LowEnergyEMOption::kLivermore :
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

  if (fConstructOptical) {
    this->ConstructOptical();
    this->ConstructCerenkov();
  }
  else {
    RMGLog::Out(RMGLog::detail, "Processes for optical photons are inactivated");
  }

  // Add decays
  RMGLog::Out(RMGLog::detail, "Adding radioactive decay physics");
  auto decay_physics = new G4DecayPhysics(G4VModularPhysicsList::verboseLevel);
  decay_physics->ConstructProcess();
  auto rad_decay_physics = new G4RadioactiveDecayPhysics(G4VModularPhysicsList::verboseLevel);
  rad_decay_physics->ConstructProcess();
  const auto the_ion_table = G4ParticleTable::GetParticleTable()->GetIonTable();
  RMGLog::Out(RMGLog::detail, "Entries in ion table "+ G4String(the_ion_table->Entries()));

  // Assign manually triton decay
  /*
  for (G4int i = 0; i < the_ion_table->Entries(); i++) {
    auto particle = the_ion_table->GetParticle(i);
    // assign Tritium (3H) life time given by NuDat 2.5 - A. Schubert 21 July 2010:
    // follow http://hypernews.slac.stanford.edu/HyperNews/geant4/get/hadronprocess/1538/1.html

    if (particle == G4Triton::Definition()) {
      RMGLog::Out(RMGLog::detail, "Adding triton decay manually");
      particle->SetPDGLifeTime(12.32*log(2.0)*365*24*3600*u::second);
      particle->SetPDGStable(false);
      auto proc_manager = particle->GetProcessManager();
      // Remove G4Decay process, which requires a registered decay table
      G4VProcess* decay_proc = nullptr;
      auto pvec = proc_manager->GetAtRestProcessVector();
      for (size_t j = 0; j < pvec->size() && decay_proc == nullptr; j++) {
        if ((*pvec)[j]->GetProcessName() == "Decay") decay_proc = (*pvec)[j];
      }
      if (decay_proc) proc_manager->RemoveProcess(decay_proc);
      // Attach RDM, which is a rest-discrete process
      proc_manager->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
      proc_manager->AddProcess(new G4RadioactiveDecayBase(), 1000, -1, 1000);
    }
  }
  */
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::AddTransportation() {

  RMGLog::Out(RMGLog::detail, "Adding transportation");

  G4VUserPhysicsList::AddTransportation();

  GetParticleIterator()->reset();
  while ((*GetParticleIterator())()) {
    auto particle = GetParticleIterator()->value();
    auto proc_manager = particle->GetProcessManager();
    auto particle_name = particle->GetParticleName();
    // step limits
    if (fLimitSteps[particle_name]) {
      proc_manager->AddProcess(new G4StepLimiter, -1, -1, 3);
      RMGLog::Out(RMGLog::detail, "Steps will be limited for ", particle_name);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::AddParallelWorldScoring() {

  RMGLog::Out(RMGLog::detail, "Adding parallel world scoring");

  auto parallel_world_scoring_proc = new G4ParallelWorldScoringProcess("ParallelWorldScoringProc");
  parallel_world_scoring_proc->SetParallelWorld("ParallelSamplingWorld");

  GetParticleIterator()->reset();
  while((*GetParticleIterator())()) {
    auto particle = GetParticleIterator()->value();
    if (!particle->IsShortLived()) {
      auto proc_manager = particle->GetProcessManager();
      proc_manager->AddProcess(parallel_world_scoring_proc);
      proc_manager->SetProcessOrderingToLast(parallel_world_scoring_proc, G4ProcessVectorDoItIndex::idxAtRest);
      proc_manager->SetProcessOrdering(parallel_world_scoring_proc, G4ProcessVectorDoItIndex::idxAlongStep, 1);
      proc_manager->SetProcessOrderingToLast(parallel_world_scoring_proc, G4ProcessVectorDoItIndex::idxPostStep);
    }
  }
}

/** Optical Processes
 *
 * The default scintillation process (see LAr properties in RMGGerdaLocalMaterialTable.cc)
 * is the one for electrons and gammas, for alphas and nuclar recoils we define two
 * additional processes, to be able to set different scintillation yields and yield ratios.
 *
 * Recap:
 * Relative scintillation yields:
 * - flat-top particles: 1
 * - electrons and gammas: 0.8
 * - alphas: 0.7
 * - nuclear recoils: 0.2-0.4
 *
 * reference: http://iopscience.iop.org/article/10.1143/JJAP.41.1538/pdf
 *
 * yield ratio:
 * - electrons and gammas: 0.23
 * - nuclear recoils: 0.75
 *
 * reference: WArP data
 */

void RMGProcessesList::ConstructOptical() {

  RMGLog::Out(RMGLog::detail, "Adding optical physics");

  // default scintillation process (electrons and gammas)
  auto scint_proc_default = new G4Scintillation("Scintillation");
  scint_proc_default->SetTrackSecondariesFirst(true);
  scint_proc_default->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  // scintillation process for alphas:
  auto scint_proc_alpha = new G4Scintillation("Scintillation");
  scint_proc_alpha->SetTrackSecondariesFirst(true);
  scint_proc_alpha->SetScintillationYieldFactor(0.875);
  scint_proc_alpha->SetScintillationExcitationRatio(1.0); // this is a guess
  scint_proc_alpha->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  // scintillation process for heavy nuclei
  auto scint_proc_nuclei = new G4Scintillation("Scintillation");
  scint_proc_nuclei->SetTrackSecondariesFirst(true);
  scint_proc_nuclei->SetScintillationYieldFactor(0.375);
  scint_proc_nuclei->SetScintillationExcitationRatio(0.75);
  scint_proc_nuclei->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  // optical processes
  auto absorption_proc     = new G4OpAbsorption();
  auto boundary_proc       = new G4OpBoundaryProcess();
  auto rayleigh_scatt_proc = new G4OpRayleigh();
  auto wls_proc            = new G4OpWLS();

  absorption_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  boundary_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  wls_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  GetParticleIterator()->reset();
  while((*GetParticleIterator())()) {
    auto particle = GetParticleIterator()->value();
    auto proc_manager = particle->GetProcessManager();
    auto particle_name = particle->GetParticleName();
     if (scint_proc_default->IsApplicable(*particle)) {
      if (particle->GetParticleName() == "GenericIon") {
        proc_manager->AddProcess(scint_proc_nuclei);
        proc_manager->SetProcessOrderingToLast(scint_proc_nuclei, G4ProcessVectorDoItIndex::idxAtRest);
        proc_manager->SetProcessOrderingToLast(scint_proc_nuclei, G4ProcessVectorDoItIndex::idxPostStep);
      } else if (particle->GetParticleName() == "alpha") {
        proc_manager->AddProcess(scint_proc_alpha);
        proc_manager->SetProcessOrderingToLast(scint_proc_alpha, G4ProcessVectorDoItIndex::idxAtRest);
        proc_manager->SetProcessOrderingToLast(scint_proc_alpha, G4ProcessVectorDoItIndex::idxPostStep);
      } else {
        proc_manager->AddProcess(scint_proc_default);
        proc_manager->SetProcessOrderingToLast(scint_proc_default, G4ProcessVectorDoItIndex::idxAtRest);
        proc_manager->SetProcessOrderingToLast(scint_proc_default, G4ProcessVectorDoItIndex::idxPostStep);
      }
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

void RMGProcessesList::SetCuts() {

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

void RMGProcessesList::SetPhysicsRealm(PhysicsRealm realm) {
  switch (realm) {
    case RMGProcessesList::PhysicsRealm::kDoubleBetaDecay :
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

    case RMGProcessesList::PhysicsRealm::kDarkMatter :
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

    case RMGProcessesList::PhysicsRealm::kCosmicRays :
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

    case RMGProcessesList::PhysicsRealm::kLArScintillation :
      RMGLog::Out(RMGLog::warning, "LAr scintillation realm unimplemented");
  }

  this->SetCuts();
  fPhysicsRealm = realm;
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::ConstructCerenkov() {

  RMGLog::Out(RMGLog::detail, "Adding Cherenkov physics");

  auto cerenkov_process = new G4Cerenkov();
  G4ProcessManager* proc_manager = nullptr;

  GetParticleIterator()->reset();
  while ((*GetParticleIterator())()) {
    auto particle = GetParticleIterator()->value();
    proc_manager = particle->GetProcessManager();
    if (cerenkov_process->IsApplicable(*particle)) {
      proc_manager->AddProcess(cerenkov_process);
      proc_manager->SetProcessOrdering(cerenkov_process, G4ProcessVectorDoItIndex::idxPostStep);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::SetLowEnergyEMOptionString(G4String option) {
  try { fLowEnergyEMOption = RMGTools::ToEnum<RMGProcessesList::LowEnergyEMOption>(option, "low energy EM option"); }
  catch (const std::bad_cast&) { return; }
}

void RMGProcessesList::SetPhysicsRealmString(G4String realm) {
  try { this->SetPhysicsRealm(RMGTools::ToEnum<RMGProcessesList::PhysicsRealm>(realm, "physics realm")); }
  catch (const std::bad_cast&) { return; }
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGProcessesList::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Processes/",
      "Commands for controlling physics processes");

  fMessenger->DeclareMethod("Realm", &RMGProcessesList::SetPhysicsRealmString)
    .SetGuidance("Set simulation realm (cut values for particles in (sensitive) detector")
    .SetParameterName("realm", false)
    .SetCandidates(RMGTools::GetCandidates<RMGProcessesList::PhysicsRealm>())
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareProperty("OpticalPhysics", fConstructOptical)
    .SetGuidance("Add optical processes to the physics list")
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("LowEnergyEMPhysics", &RMGProcessesList::SetLowEnergyEMOptionString)
    .SetGuidance("Add low energy electromagnetic processes to the physics list")
    .SetCandidates(RMGTools::GetCandidates<RMGProcessesList::LowEnergyEMOption>())
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("EnableGammaAngularCorrelation", &RMGProcessesList::SetUseGammaAngCorr)
    .SetGuidance("")
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("GammaTwoJMAX", &RMGProcessesList::SetGammaTwoJMAX)
    .SetGuidance("")
    .SetParameterName("x", false)
    .SetRange("x > 0")
    .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("StoreICLevelData", &RMGProcessesList::SetStoreICLevelData)
    .SetGuidance("")
    .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: shiftwidth=2 tabstop=2 expandtab

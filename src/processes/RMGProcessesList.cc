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
#include "G4RadioactiveDecay.hh"
#include "G4IonTable.hh"
#include "G4Scintillation.hh"
#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4OpWLS.hh"
#include "G4Cerenkov.hh"

#include "RMGProcessesMessenger.hh"
#include "RMGLog.hh"

RMGProcessesList::RMGProcessesList() :
  G4VModularPhysicsList() {

  fProcessesMessenger = std::unique_ptr<RMGProcessesMessenger>(new RMGProcessesMessenger(this));

  // The default values for the energy thresholds are tuned to 100 keV
  // in natural germanium (i.e., the BBdecay realm)
  G4VUserPhysicsList::defaultCutValue = 0.1*CLHEP::mm;
  fCutForGamma                        = 0.1*CLHEP::mm;
  fCutForElectron                     = 0.1*CLHEP::mm;
  fCutForPositron                     = 0.1*CLHEP::mm;
  fCutForGammaSensitive               = 0.1*CLHEP::mm;
  fCutForElectronSensitive            = 0.1*CLHEP::mm;
  fCutForPositronSensitive            = 0.1*CLHEP::mm;
  fCutForProton                       = 0.;
  fCutForAlpha                        = 0.;
  fCutForGenericIon                   = 0.;

  this->SetCuts();

  G4VModularPhysicsList::verboseLevel = 0;
  this->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  fUseLowEnergy       = true;
  fUseLowEnergyOption = 0;
  fConstructOptical   = false;
  fUseOpticalPhysOnly = false;

  fPhysicsListHadrons = " ";
}

void RMGProcessesList::SetUseAngCorr(G4int max_two_j) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetCorrelatedGamma(max_two_j);
  // The above method takes a bool, so maxTwoJ=0 will disable
  pars->SetTwoJMAX(max_two_j);
}

// Set Store IC Level Data
void RMGProcessesList::SetStoreICLevelData(G4bool store) {
  auto pars = G4NuclearLevelData::GetInstance()->GetParameters();
  pars->SetStoreICLevelData(store);
}

void RMGProcessesList::ConstructParticle() {

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

void RMGProcessesList::ConstructProcess() {

  this->AddTransportation();

  // parallel worlds must be added after G4Transportation
  if (G4RunManagerKernel::GetRunManagerKernel()->GetNumberOfParallelWorld() > 0) {
    AddParallelWorldScoring();
  }

  if (fUseOpticalPhysOnly) {
    RMGLog::Out(RMGLog::detail, "Constucting optical processes");
    ConstructOp();
    RMGLog::Out(RMGLog::detail, "Constucting Cerenkov processes");
    ConstructCerenkov();
    return;
  }

  // EM Physics
  G4VPhysicsConstructor* em_constructor = nullptr;
  if (fUseLowEnergy) {
    switch (fUseLowEnergyOption){
      // from https://geant4.web.cern.ch/node/1731
      case 1:
        em_constructor = new G4EmStandardPhysics_option1(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 1");
        break;
      case 2:
        em_constructor = new G4EmStandardPhysics_option2(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 2");
        break;
      case 3:
        em_constructor = new G4EmStandardPhysics_option3(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 3");
        break;
      case 4:
        em_constructor = new G4EmStandardPhysics_option4(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 4");
        break;
      case 5:
        em_constructor = new G4EmPenelopePhysics(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using Penelope Physics");
        break;
      case 6:
        em_constructor = new G4EmLivermorePolarizedPhysics(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using Livermore-Polarized Physics");
        break;
      default:
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
  auto em_extra_physics = new G4EmExtraPhysics(G4VModularPhysicsList::verboseLevel);
  G4String choice = "on";
  em_extra_physics->Synch(choice);
  em_extra_physics->GammaNuclear(choice);
  em_extra_physics->MuonNuclear(choice);
  em_extra_physics->ConstructProcess();

  if (fConstructOptical) {
    RMGLog::Out(RMGLog::detail, "Constucting optical processes");
    this->ConstructOp();
    RMGLog::Out(RMGLog::detail, "Constucting cerenkov processes");
    this->ConstructCerenkov();
  }
  else {
    RMGLog::Out(RMGLog::summary, "Processes for Optical Photons are inactivated");
  }
  RMGLog::Out(RMGLog::detail, "Finished optical contstruction physics");

  // Add decays
  auto decay_physics = new G4DecayPhysics(G4VModularPhysicsList::verboseLevel);
  decay_physics->ConstructProcess();
  auto rad_decay_physics = new G4RadioactiveDecayPhysics(G4VModularPhysicsList::verboseLevel);
  rad_decay_physics->ConstructProcess();
  RMGLog::Out(RMGLog::detail, "finished decays processes construction");
  const G4IonTable* the_ion_table = G4ParticleTable::GetParticleTable()->GetIonTable();
  RMGLog::Out(RMGLog::detail, "entries in ion table "+ G4String(the_ion_table->Entries()));

  // Assign manually triton decay
  for (G4int i = 0; i < the_ion_table->Entries(); i++) {
    auto particle = the_ion_table->GetParticle(i);
    // assign Tritium (3H) life time given by NuDat 2.5 - A. Schubert 21 July 2010:
    // follow http://hypernews.slac.stanford.edu/HyperNews/geant4/get/hadronprocess/1538/1.html

    if (particle == G4Triton::Definition()) {
      RMGLog::Out(RMGLog::detail, " found trition particle ");
      particle->SetPDGLifeTime(12.32*log(2.0)*365*24*3600*CLHEP::second);
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
      proc_manager->AddProcess(new G4RadioactiveDecay(), 1000, -1, 1000);
    }
  }
  RMGLog::Out(RMGLog::detail, "finsihed triton decay processes construction");

  this->DumpPhysicsList();

  // FIXME: is this really needed?
  // the following logic emulates the /process/setVerbose [VerboseLevel] all
  // auto pProcNameVec = G4ProcessTable::GetProcessTable()->GetNameList();
  // for (auto& i : *pProcNameVec) {
  //   auto v = G4ProcessTable::GetProcessTable()->FindProcesses(i);
  //   (*v)[0]->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  //   delete v;
  // }
}

void RMGProcessesList::AddTransportation() {

  G4VUserPhysicsList::AddTransportation();

  GetParticleIterator()->reset();
  while ((*GetParticleIterator())()) {
    auto particle = GetParticleIterator()->value();
    auto proc_manager = particle->GetProcessManager();
    auto particle_name = particle->GetParticleName();
    // step limits
    if(fLimitSteps[particle_name]) {
      proc_manager->AddProcess(new G4StepLimiter, -1, -1, 3);
      RMGLog::Out(RMGLog::detail, "Steps will be limited for ", particle_name);
    }
  }
}

void RMGProcessesList::AddParallelWorldScoring() {
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

void RMGProcessesList::ConstructOp() {

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

// Hadronic processes

void RMGProcessesList::SetCuts() {

  G4HadronicProcessStore::Instance()->SetVerbose(G4VModularPhysicsList::verboseLevel);
  // special for low energy physics
  G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(250*CLHEP::eV, 100.*CLHEP::GeV);

  this->SetCutValue(fCutForGamma, "gamma");
  this->SetCutValue(fCutForElectron, "e-");
  this->SetCutValue(fCutForPositron, "e+");
  this->SetCutValue(fCutForProton, "proton");
  this->SetCutValue(fCutForAlpha, "alpha");
  this->SetCutValue(fCutForGenericIon, "GenericIon");

  if (G4RegionStore::GetInstance()) {
    if (G4RegionStore::GetInstance()->size() > 1) {
      // Set different cuts for the sensitive region
      auto region = G4RegionStore::GetInstance()->GetRegion("SensitiveRegion", false);
      if (region) {
        RMGLog::Out(RMGLog::detail, "Register cuts for SensitiveRegion ");
        auto cuts = region->GetProductionCuts();
        if (!cuts) cuts = new G4ProductionCuts; // zero pointer --> cuts not defined yet
        cuts->SetProductionCut(fCutForGammaSensitive, "gamma");
        cuts->SetProductionCut(fCutForElectronSensitive, "e-");
        cuts->SetProductionCut(fCutForPositronSensitive, "e+");
        cuts->SetProductionCut(G4VUserPhysicsList::defaultCutValue, "proton");
        cuts->SetProductionCut(G4VUserPhysicsList::defaultCutValue, "alpha");
        cuts->SetProductionCut(G4VUserPhysicsList::defaultCutValue, "GenericIon");
        region->SetProductionCuts(cuts);
      }
    }
  }
  RMGLog::Out(RMGLog::detail, "Production cuts set");
}

void RMGProcessesList::SetRealm(G4String realm) {
  if (realm == "BBdecay") {
    RMGLog::Out(RMGLog::summary, "Realm set to BBdecay");
    fCutForGamma             = 0.1*CLHEP::mm;
    fCutForElectron          = 0.1*CLHEP::mm;
    fCutForPositron          = 0.1*CLHEP::mm;
    fCutForProton            = G4VUserPhysicsList::defaultCutValue;
    fCutForAlpha             = G4VUserPhysicsList::defaultCutValue;
    fCutForGenericIon        = G4VUserPhysicsList::defaultCutValue;
    fCutForGammaSensitive    = fCutForGamma;
    fCutForElectronSensitive = fCutForElectron;
    fCutForPositronSensitive = fCutForPositron;
    this->SetCuts();
  }
  else if (realm == "DarkMatter") {
    RMGLog::Out(RMGLog::summary, "Realm set to DarkMatter");
    // These values are tuned to ~1 keV for gamma, e+, e- in
    // natural germanium.
    fCutForGamma             = 0.005*CLHEP::mm;
    fCutForElectron          = 0.0005*CLHEP::mm;
    fCutForPositron          = 0.0005*CLHEP::mm;
    fCutForProton            = G4VUserPhysicsList::defaultCutValue;
    fCutForAlpha             = G4VUserPhysicsList::defaultCutValue;
    fCutForGenericIon        = G4VUserPhysicsList::defaultCutValue;
    fCutForGammaSensitive    = fCutForGamma;
    fCutForElectronSensitive = fCutForElectron;
    fCutForPositronSensitive = fCutForPositron;
    this->SetCuts();
  }
  else if (realm == "CosmicRays") {
    RMGLog::Out(RMGLog::summary, "Realm set to CosmicRays (cut-per-region)");
    fCutForGamma             = 5*CLHEP::cm;
    fCutForElectron          = 1*CLHEP::cm;
    fCutForPositron          = 1*CLHEP::cm;
    fCutForProton            = 5*CLHEP::mm;
    fCutForAlpha             = 5*CLHEP::mm;
    fCutForGenericIon        = 5*CLHEP::mm;
    fCutForGammaSensitive    = 30*CLHEP::mm;
    fCutForElectronSensitive = 0.04*CLHEP::mm;
    fCutForPositronSensitive = 0.04*CLHEP::mm;
    this->SetCuts();
  }
  else {
    RMGLog::Out(RMGLog::error, "Error: invalid energy cut realm \"" + realm + "\"." +
                    "Must use either \"BBdecay\" or \"DarkMatter\".");
  }
}

void RMGProcessesList::ConstructCerenkov() {
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

void RMGProcessesList::DumpPhysicsList() {
//   RMGLog::Out(RMGLog::detail, "====================================================================");
//   RMGLog::Out(RMGLog::detail, "                      MaGe physics list                             ");
//   RMGLog::Out(RMGLog::detail, "====================================================================");
//   if (useLowE)  RMGLog::Out(RMGLog::detail, "Electromagnetic physics: Livermore/LowEnergy ");
//   else RMGLog::Out(RMGLog::detail, "Electromagnetic physics: Standard ");
//   RMGLog::Out(RMGLog::detail, "====================================================================");

//   if (constructOptical) RMGLog::Out(RMGLog::detail, "Physics for optical photons registered");
//   else RMGLog::Out(RMGLog::detail, "No processes activated for optical photons");
//   RMGLog::Out(RMGLog::detail, "====================================================================");

//   if (fUseNoHadPhysFlag) {
//     RMGLog::Out(RMGLog::detail, "No processes activated for hadrons");
//     RMGLog::Out(RMGLog::detail, "====================================================================");
//     return;
//   }
//   RMGLog::Out(RMGLog::detail, physics_list_hadrons_);
//   RMGLog::Out(RMGLog::detail, "====================================================================");
}

void RMGProcessesList::GetStepLimits() {
  this->DumpPhysicsList();
  RMGLog::Out(RMGLog::detail, "========================Show limits ================================");
  RMGLog::Out(RMGLog::detail, " gamma " + G4String(G4VModularPhysicsList::GetCutValue("gamma")) + "keV");
  RMGLog::Out(RMGLog::detail, " e-    " + G4String(GetCutValue("e-")) + "keV");
  RMGLog::Out(RMGLog::detail, " e+    " + G4String(GetCutValue("e+")) + "keV");
  RMGLog::Out(RMGLog::detail, " p     " + G4String(GetCutValue("proton")) + "keV");
  RMGLog::Out(RMGLog::detail, " alpha " + G4String(GetCutValue("alpha")) +"keV");
  RMGLog::Out(RMGLog::detail, " Ion   " + G4String(GetCutValue("GenericIon")) + "keV");
  RMGLog::Out(RMGLog::detail, "====================================================================");
}

// vim: shiftwidth=2 tabstop=2 expandtab

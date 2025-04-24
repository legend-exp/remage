// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "RMGPhysics.hh"

#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4Cerenkov.hh"
#include "G4DecayPhysics.hh"
#include "G4DeexPrecoParameters.hh"
#include "G4EmExtraPhysics.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmLivermorePolarizedPhysics.hh"
#include "G4EmParameters.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4EmStandardPhysics_option2.hh"
#include "G4EmStandardPhysics_option3.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronElasticProcess.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4HadronPhysicsQGSP_BERT_HP.hh"
#include "G4HadronPhysicsQGSP_BIC_AllHP.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"
#include "G4HadronPhysicsShielding.hh"
#include "G4HadronicParameters.hh"
#include "G4HadronicProcessStore.hh"
#include "G4IonConstructor.hh"
#include "G4IonPhysics.hh"
#include "G4IonTable.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4NeutronCaptureProcess.hh"
#include "G4NuclearLevelData.hh"
#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4OpWLS.hh"
#include "G4OpticalParameters.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleHPCaptureData.hh"
#include "G4ParticleHPElastic.hh"
#include "G4ParticleHPElasticData.hh"
#include "G4ParticleHPThermalScattering.hh"
#include "G4ParticleHPThermalScatteringData.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessTable.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4RegionStore.hh"
#include "G4RunManagerKernel.hh"
#include "G4Scintillation.hh"
#include "G4ShortLivedConstructor.hh"
#include "G4StepLimiter.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4ThermalNeutrons.hh"

#include "RMGConfig.hh"
#include "RMGLog.hh"
#include "RMGNeutronCaptureProcess.hh"
#include "RMGOpWLSProcess.hh"
#include "RMGTools.hh"

namespace u = CLHEP;

RMGPhysics::RMGPhysics() {

  G4VUserPhysicsList::defaultCutValue = 0.1 * u::mm;
  this->SetPhysicsRealm(PhysicsRealm::kDoubleBetaDecay);

  G4VModularPhysicsList::verboseLevel = RMGLog::GetLogLevel() <= RMGLog::debug ? 1 : 0;
  // need to set this here for it to take effect...?
  G4HadronicParameters::Instance()->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

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

  G4BosonConstructor::ConstructParticle();
  G4LeptonConstructor::ConstructParticle();
  G4MesonConstructor::ConstructParticle();
  G4BaryonConstructor::ConstructParticle();
  G4IonConstructor::ConstructParticle();
  G4ShortLivedConstructor::ConstructParticle();
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::ConstructProcess() {

  G4VUserPhysicsList::AddTransportation();


  // EM Physics
  G4VPhysicsConstructor* em_constructor = nullptr;
  RMGLog::Out(RMGLog::detail, "Adding electromagnetic physics");
  switch (fLowEnergyEMOption) {
    case LowEnergyEMOption::kNone:
      RMGLog::Out(RMGLog::detail, "Using Standard electromagnetic physics");
      em_constructor = new G4EmStandardPhysics(G4VModularPhysicsList::verboseLevel);
      break;
    // from https://geant4.web.cern.ch/node/1731
    case LowEnergyEMOption::kOption1:
      em_constructor = new G4EmStandardPhysics_option1(G4VModularPhysicsList::verboseLevel);
      RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 1");
      break;
    case LowEnergyEMOption::kOption2:
      em_constructor = new G4EmStandardPhysics_option2(G4VModularPhysicsList::verboseLevel);
      RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 2");
      break;
    case LowEnergyEMOption::kOption3:
      em_constructor = new G4EmStandardPhysics_option3(G4VModularPhysicsList::verboseLevel);
      RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 3");
      break;
    case LowEnergyEMOption::kOption4:
      em_constructor = new G4EmStandardPhysics_option4(G4VModularPhysicsList::verboseLevel);
      RMGLog::Out(RMGLog::detail, "Using EmPhysics Option 4");
      break;
    case LowEnergyEMOption::kPenelope:
      em_constructor = new G4EmPenelopePhysics(G4VModularPhysicsList::verboseLevel);
      RMGLog::Out(RMGLog::detail, "Using Penelope Physics");
      break;
    case LowEnergyEMOption::kLivermorePolarized:
      em_constructor = new G4EmLivermorePolarizedPhysics(G4VModularPhysicsList::verboseLevel);
      RMGLog::Out(RMGLog::detail, "Using Livermore-Polarized Physics");
      break;
    case LowEnergyEMOption::kLivermore:
      RMGLog::Out(RMGLog::detail, "Using Livermore/LowEnergy electromagnetic physics");
      em_constructor = new G4EmLivermorePhysics(G4VModularPhysicsList::verboseLevel);
      break;
  }

  em_constructor->ConstructProcess();


  // Includes synchrotron radiation, gamma-nuclear, muon-nuclear and
  // e+/e- nuclear interactions
  RMGLog::Out(RMGLog::detail, "Adding extra electromagnetic physics");
  auto em_extra_physics = new G4EmExtraPhysics(G4VModularPhysicsList::verboseLevel);
  em_extra_physics->Synch(true);
  em_extra_physics->GammaNuclear(true);
  em_extra_physics->MuonNuclear(true);
  em_extra_physics->ConstructProcess();


  // G4EmExtraPhysics does not propagate the verbose level...
  auto synch_proc = G4ProcessTable::GetProcessTable()->FindProcesses("SynRad");
  for (size_t i = 0; i < synch_proc->size(); i++) {
    (*synch_proc)[(int)i]->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  }

  if (fConstructOptical) this->ConstructOptical();
  else RMGLog::Out(RMGLog::detail, "Processes for optical photons are inactivated");

  // Hadronic Physics

  // Geant4 11.2 changed this to 1 year, which is rather short for our use case.
  // Reset it to the old default of Geant4 11.0/11.1.
  // note: this still might be too short for very rare decays, but it can be increased with
  // a macro command.
  G4HadronicParameters::Instance()->SetTimeThresholdForRadioactiveDecay(1.0e+27 * u::ns);

  /*
  G4ParticleHPManager::GetInstance()->SetSkipMissingIsotopes( false );
  G4ParticleHPManager::GetInstance()->SetDoNotAdjustFinalState( true );
  G4ParticleHPManager::GetInstance()->SetUseOnlyPhotoEvaporation( true );
  G4ParticleHPManager::GetInstance()->SetNeglectDoppler( false );
  G4ParticleHPManager::GetInstance()->SetProduceFissionFragments( false );
  G4ParticleHPManager::GetInstance()->SetUseWendtFissionModel( false );
  G4ParticleHPManager::GetInstance()->SetUseNRESP71Model( false );
  */

  if (fHadronicPhysicsListOption != HadronicPhysicsListOption::kNone) {
    RMGLog::Out(RMGLog::detail, "Adding hadronic elastic physics");
    G4VPhysicsConstructor* hElasticPhysics =
        new G4HadronElasticPhysicsHP(G4VModularPhysicsList::verboseLevel);
    hElasticPhysics->ConstructProcess();

    if (fUseThermalScattering) {
      RMGLog::Out(RMGLog::detail, "Adding neutron thermal scattering elastic physics");
      G4VPhysicsConstructor* hThermalScatteringPhysics =
          new G4ThermalNeutrons(G4VModularPhysicsList::verboseLevel);
      hThermalScatteringPhysics->ConstructProcess();
    }

    G4VPhysicsConstructor* hPhysics = nullptr;
    switch (fHadronicPhysicsListOption) {
      case HadronicPhysicsListOption::kNone:
        throw std::domain_error("Got unexpected HadronicPhysicsListOption::kNone");
      case HadronicPhysicsListOption::kQGSP_BIC_HP:
        hPhysics = new G4HadronPhysicsQGSP_BIC_HP(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using QGSP_BIC_HP");
        break;
      case HadronicPhysicsListOption::kQGSP_BERT_HP:
        hPhysics = new G4HadronPhysicsQGSP_BERT_HP(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using QGSP_BERT_HP");
        break;
      case HadronicPhysicsListOption::kFTFP_BERT_HP:
        hPhysics = new G4HadronPhysicsFTFP_BERT_HP(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using FTFP_BERT_HP");
        break;
      case HadronicPhysicsListOption::kShielding:
        hPhysics = new G4HadronPhysicsShielding(G4VModularPhysicsList::verboseLevel);
        RMGLog::Out(RMGLog::detail, "Using Shielding");
        break;
    }
    RMGLog::Out(RMGLog::detail, "Adding hadronic inelastic physics");
    hPhysics->ConstructProcess();

    if (fUseGrabmayrGammaCascades) {
      // Apply RMG custom neutron capture
      // Mostly similar to examples/extended/Hadr04
      auto pManager = G4Neutron::Neutron()->GetProcessManager();
      // Find the existing neutron capture process
      auto neutronCaptureProcess =
          dynamic_cast<G4NeutronCaptureProcess*>(pManager->GetProcess("nCapture"));

      // Overwrite the old Process, keeping all of the interactions
      if (neutronCaptureProcess) {
        RMGLog::Out(RMGLog::detail, "Overwriting NeutronCaptureProcess");
        pManager->RemoveProcess(neutronCaptureProcess);
        auto RMGProcess = new RMGNeutronCaptureProcess();
        // HP cross section data set not naturally in G4NeutronCaptureProcess
        auto dataSet = new G4ParticleHPCaptureData();
        RMGProcess->AddDataSet(dataSet);
        // Move all interactions to the new process
        for (auto& el : neutronCaptureProcess->GetHadronicInteractionList()) {
          RMGProcess->RegisterMe(el);
        }
        pManager->AddDiscreteProcess(RMGProcess);
      } else {
        RMGLog::Out(RMGLog::error, "Could not apply custom neutron capture model");
      }
    }

    RMGLog::Out(RMGLog::detail, "Adding stopping physics");
    G4VPhysicsConstructor* stoppingPhysics =
        new G4StoppingPhysics(G4VModularPhysicsList::verboseLevel);
    stoppingPhysics->ConstructProcess();

    RMGLog::Out(RMGLog::detail, "Adding ion physics");
    G4VPhysicsConstructor* ionPhysics = new G4IonPhysics(G4VModularPhysicsList::verboseLevel);
    ionPhysics->ConstructProcess();
  }

  // Add decays
  RMGLog::Out(RMGLog::detail, "Adding radioactive decay physics");
  auto decay_physics = new G4DecayPhysics(G4VModularPhysicsList::verboseLevel);
  decay_physics->ConstructProcess();
  auto rad_decay_physics = new G4RadioactiveDecayPhysics(G4VModularPhysicsList::verboseLevel);
  rad_decay_physics->ConstructProcess();
  const auto the_ion_table = G4ParticleTable::GetParticleTable()->GetIonTable();
  RMGLog::Out(RMGLog::detail, "Entries in ion table ", the_ion_table->Entries());

  // add step limits
  auto step_limits = new G4StepLimiterPhysics();
  step_limits->ConstructProcess();
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
  auto absorption_proc = new G4OpAbsorption();
  auto boundary_proc = new G4OpBoundaryProcess();
  auto rayleigh_scatt_proc = new G4OpRayleigh();
  G4VProcess* wls_proc = new G4OpWLS();
  auto cerenkov_proc = new G4Cerenkov();

  absorption_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  boundary_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);
  wls_proc->SetVerboseLevel(G4VModularPhysicsList::verboseLevel);

  if (fUseOpticalCustomWLS) {
    auto wls_proc_wrapped = new RMGOpWLSProcess();
    wls_proc_wrapped->RegisterProcess(wls_proc);
    wls_proc = wls_proc_wrapped;
  }

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

void RMGPhysics::SetCuts() {

  RMGLog::Out(RMGLog::debug, "Setting particle cut values");

  // ask G4HadronicProcessStore to respect the global verbosity level
  G4HadronicProcessStore::Instance()->SetVerbose(G4VModularPhysicsList::verboseLevel);

  // default production thresholds for the world volume
  this->SetCutsWithDefault();

  // special for low energy physics
  G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(fLowEnergyRange, fHighEnergyRange);

  // set cut values for the default region (world volume)
  this->SetCutValue(fProdCuts.gamma, "gamma");
  this->SetCutValue(fProdCuts.electron, "e-");
  this->SetCutValue(fProdCuts.positron, "e+");
  this->SetCutValue(fProdCuts.proton, "proton");

  // set different cuts for the sensitive region
  // second argument is verbosity, setting to false to avoid warning printout
  // if region is not found. we are going to check ourselves
  auto region = G4RegionStore::GetInstance()->GetRegion("SensitiveRegion", false);
  if (region) {
    RMGLog::Out(RMGLog::detail, "Register cuts for G4Region 'SensitiveRegion'");
    auto cuts = region->GetProductionCuts();
    if (!cuts) cuts = new G4ProductionCuts;
    cuts->SetProductionCut(fProdCutsSensitive.gamma, "gamma");
    cuts->SetProductionCut(fProdCutsSensitive.electron, "e-");
    cuts->SetProductionCut(fProdCutsSensitive.positron, "e+");
    cuts->SetProductionCut(fProdCutsSensitive.proton, "proton");
    region->SetProductionCuts(cuts);
  } else {
    RMGLog::Out(RMGLog::warning, "Could not find G4Region 'SensitiveRegion' in the store. ",
        "No special production cuts applied");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetPhysicsRealm(PhysicsRealm realm) {
  switch (realm) {
    case PhysicsRealm::kDoubleBetaDecay:
      RMGLog::Out(RMGLog::summary, "Realm set to DoubleBetaDecay");

      fProdCuts = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
      fProdCuts.gamma = 0.1 * u::mm;
      fProdCuts.electron = 0.1 * u::mm;
      fProdCuts.positron = 0.1 * u::mm;

      fProdCutsSensitive = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
      fProdCutsSensitive.gamma = 0.1 * u::mm;
      fProdCutsSensitive.electron = 0.1 * u::mm;
      fProdCutsSensitive.positron = 0.1 * u::mm;
      break;

    case PhysicsRealm::kDarkMatter:
      RMGLog::Out(RMGLog::summary, "Realm set to DarkMatter");

      fProdCuts = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
      fProdCuts.gamma = 5 * u::um;
      fProdCuts.electron = 0.5 * u::um;
      fProdCuts.positron = 0.5 * u::um;

      fProdCutsSensitive = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
      fProdCutsSensitive.gamma = 5 * u::um;
      fProdCutsSensitive.electron = 0.5 * u::um;
      fProdCutsSensitive.positron = 0.5 * u::um;
      break;

    case PhysicsRealm::kCosmicRays:
      RMGLog::Out(RMGLog::summary, "Realm set to CosmicRays (cut-per-region)");
      fProdCuts = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
      fProdCuts.gamma = 5 * u::cm;
      fProdCuts.electron = 1 * u::cm;
      fProdCuts.positron = 1 * u::cm;
      fProdCuts.proton = 5 * u::mm;

      fProdCutsSensitive = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
      fProdCutsSensitive.gamma = 30 * u::mm;
      fProdCutsSensitive.electron = 40 * u::um;
      fProdCutsSensitive.positron = 40 * u::um;
      break;

    case PhysicsRealm::kLArScintillation:
      RMGLog::Out(RMGLog::warning, "LAr scintillation realm unimplemented");
      break;
    case PhysicsRealm::kUserDefined:
      RMGLog::Out(RMGLog::summary,
          "User requested custom physics realm, based on directly setting production cuts.");

      break;
  }

  this->SetCuts();
  fPhysicsRealm = realm;
}


////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetSensitiveProductionCut(double cut) {

  if (this->fPhysicsRealmSet) {
    RMGLog::Out(RMGLog::warning, "Setting the production cuts for the sensitive region",
        " while the realm has already been explicitly set, this will override the production cuts");
  }

  RMGLog::OutFormat(RMGLog::summary,
      "Setting user defined production cuts for sensitive region to {} mm", cut / u::mm);

  fProdCutsSensitive = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
  fProdCutsSensitive.gamma = cut;
  fProdCutsSensitive.electron = cut;
  fProdCutsSensitive.positron = cut;

  this->SetCuts();
  fPhysicsRealm = PhysicsRealm::kUserDefined;
}
////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::SetDefaultProductionCut(double cut) {

  if (this->fPhysicsRealmSet) {
    RMGLog::Out(RMGLog::warning, "Setting the production cuts for the sensitive region",
        " while the realm has already been explicitly set, this will override the production cuts");
  }

  RMGLog::OutFormat(RMGLog::summary,
      "Setting user defined production cuts for default region to {} mm", cut / u::mm);

  fProdCuts = ProdCutStore(G4VUserPhysicsList::defaultCutValue);
  fProdCuts.gamma = cut;
  fProdCuts.electron = cut;
  fProdCuts.positron = cut;

  this->SetCuts();
  fPhysicsRealm = PhysicsRealm::kUserDefined;
}

////////////////////////////////////////////////////////////////////////////////////////////


void RMGPhysics::SetLowEnergyEMOptionString(std::string option) {
  try {
    fLowEnergyEMOption = RMGTools::ToEnum<LowEnergyEMOption>(option, "low energy EM option");
  } catch (const std::bad_cast&) { return; }
}

void RMGPhysics::SetHadronicPhysicsListOptionString(std::string option) {
  try {
    fHadronicPhysicsListOption =
        RMGTools::ToEnum<HadronicPhysicsListOption>(option, "hadronic physics list option");
  } catch (const std::bad_cast&) { return; }
}

void RMGPhysics::SetPhysicsRealmString(std::string realm) {

  fPhysicsRealmSet = true;
  try {
    this->SetPhysicsRealm(RMGTools::ToEnum<PhysicsRealm>(realm, "physics realm"));
  } catch (const std::bad_cast&) { return; }
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGPhysics::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Processes/",
      "Commands for controlling physics processes");

  fMessenger->DeclareMethod("Realm", &RMGPhysics::SetPhysicsRealmString)
      .SetGuidance("Set simulation realm (cut values for particles in (sensitive) detector")
      .SetParameterName("realm", false)
      .SetCandidates(RMGTools::GetCandidates<PhysicsRealm>())
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit("DefaultProductionCut", "mm", &RMGPhysics::SetDefaultProductionCut)
      .SetGuidance("Set simulation production cuts, for default region for electrons, positions, "
                   "and gammas. Notes: this overrides the values from the physics realm. This does "
                   "not apply to protons, alphas or generic ions.")
      .SetParameterName("cut", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit("SensitiveProductionCut", "mm", &RMGPhysics::SetSensitiveProductionCut)
      .SetGuidance("Set simulation production cuts, for sensitive region for electrons, positions, "
                   "and gammas. Notes: this overrides the values from the physics realm. This does "
                   "not apply to protons, alphas or generic ions.")
      .SetParameterName("cut", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareProperty("OpticalPhysics", fConstructOptical)
      .SetGuidance("Add optical processes to the physics list")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareProperty("OpticalPhysicsMaxOneWLSPhoton", fUseOpticalCustomWLS)
      .SetGuidance(
          "Use a custom wavelegth shifting process that produces at maximum one secondary photon.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("LowEnergyEMPhysics", &RMGPhysics::SetLowEnergyEMOptionString)
      .SetGuidance("Add low energy electromagnetic processes to the physics list")
      .SetCandidates(RMGTools::GetCandidates<LowEnergyEMOption>())
      .SetDefaultValue(RMGTools::GetCandidate(LowEnergyEMOption::kLivermore))
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("HadronicPhysics", &RMGPhysics::SetHadronicPhysicsListOptionString)
      .SetGuidance("Add hadronic processes to the physics list")
      .SetCandidates(RMGTools::GetCandidates<HadronicPhysicsListOption>())
      .SetDefaultValue(RMGTools::GetCandidate(HadronicPhysicsListOption::kShielding))
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("ThermalScattering", &RMGPhysics::SetUseThermalScattering)
      .SetGuidance("Use thermal scattering cross sections for neutrons")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("EnableGammaAngularCorrelation", &RMGPhysics::SetUseGammaAngCorr)
      .SetGuidance("Set correlated gamma emission flag")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("GammaTwoJMAX", &RMGPhysics::SetGammaTwoJMAX)
      .SetGuidance("Set max 2J for sampling of angular correlations")
      .SetParameterName("x", false)
      .SetRange("x > 0")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("StoreICLevelData", &RMGPhysics::SetStoreICLevelData)
      .SetGuidance("Store e- internal conversion data")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareProperty("UseGrabmayrsGammaCascades", fUseGrabmayrGammaCascades)
      .SetGuidance("Use custom RMGNeutronCapture to apply Grabmayrs gamma cascades.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit);
}

// vim: shiftwidth=2 tabstop=2 expandtab

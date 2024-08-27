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

#include "RMGManager.hh"

#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#endif
#include "G4Backtrace.hh"
#include "G4GenericMessenger.hh"
#include "G4StateManager.hh"
#include "G4Threading.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VUserPhysicsList.hh"
#include "G4VisExecutive.hh"
#include "G4VisManager.hh"
#include "Randomize.hh"

#include "RMGConfig.hh"
#include "RMGExceptionHandler.hh"
#include "RMGHardware.hh"
#include "RMGPhysics.hh"
#include "RMGTools.hh"
#include "RMGUserAction.hh"
#include "RMGUserInit.hh"
#include "RMGWorkerInitialization.hh"

#if RMG_HAS_ROOT
#include "TEnv.h"
#endif

RMGManager* RMGManager::fRMGManager = nullptr;

G4ThreadLocal std::map<int, int> RMGManager::fNtupleIDs = {};

RMGManager::RMGManager(std::string app_name, int argc, char** argv)
    : fApplicationName(app_name), fArgc(argc), fArgv(argv) {

  if (fRMGManager) RMGLog::Out(RMGLog::fatal, "RMGManager must be singleton!");
  fRMGManager = this;

#if RMG_HAS_ROOT
  // turn off ROOT's stacktrace
  gEnv->SetValue("Root.Stacktrace", 0);
#endif

  // limit Geant4 stacktrace dumping to segfaults
  G4Backtrace::DefaultSignals() = std::set<int>{SIGSEGV};

  // initialize central hook for dependent applications to influence our output.
  fUserInit = std::make_shared<RMGUserInit>();
  fUserInit->RegisterDefaultOptionalOutputSchemes();

  this->DefineCommands();
}

void RMGManager::Initialize() {

  RMGLog::Out(RMGLog::detail, "Initializing application");

  if (!fG4RunManager) {
    if (fNThreads == 1) {
      this->SetUpDefaultG4RunManager(G4RunManagerType::Serial);
      RMGLog::Out(RMGLog::detail, "Execution is sequential (one-threaded)");
    } else {
      this->SetUpDefaultG4RunManager();
      if (fNThreads <= 0) fNThreads = G4Threading::G4GetNumberOfCores();
      else fNThreads = std::min(fNThreads, G4Threading::G4GetNumberOfCores());
      fG4RunManager->SetNumberOfThreads(fNThreads);
      if (!IsExecSequential())
        RMGLog::OutFormat(RMGLog::detail, "Execution is multi-threaded ({} threads are used)",
            fNThreads);
      else {
        RMGLog::OutFormat(RMGLog::warning,
            "multi-threaded execution with {} threads requested, but executing sequentially",
            fNThreads);
      }
    }
  }

  if (!fPhysicsList) this->SetUpDefaultProcessesList();
  if (!fG4VisManager) this->SetUpDefaultG4VisManager();
  fG4VisManager->Initialize();

  std::string _str;
  for (const auto& i : fG4VisManager->GetAvailableGraphicsSystems()) {
    _str += i->GetNickname() + " ";
  }
  RMGLog::Out(RMGLog::detail, "Available graphic systems: ", _str);

  if (!fUserAction) this->SetUpDefaultUserAction();
  if (!fDetectorConstruction) this->SetUpDefaultDetectorConstruction();

  fG4RunManager->SetUserInitialization(fDetectorConstruction);
  fG4RunManager->SetUserInitialization(fPhysicsList);
  fG4RunManager->SetUserInitialization(fUserAction);

  if (!fIsRandControlled) {
    SetRandSystemEntropySeed();
    fIsRandControlled = false;
  }
}

std::unique_ptr<G4UIExecutive> RMGManager::StartInteractiveSession() {

  RMGLog::Out(RMGLog::summary, "Entering interactive mode");
  auto cval = std::getenv("DISPLAY");
  auto val = cval == nullptr ? std::string("") : std::string(cval);
  if (val.empty()) RMGLog::Out(RMGLog::warning, "DISPLAY not set, forcing G4UI_USE_TCSH=1");
  return std::make_unique<G4UIExecutive>(fArgc, fArgv, val.empty() ? "tcsh" : "");
}

void RMGManager::Run() {

  // desired behavior
  // - by default (nothing is specified), open an interactive session
  // - if macro is specified, run and quit
  // - if macro is specified and fInteractive is true, do not quit afterwards
  // - a macro can request interactive mode

  // FIXME: logic here does not work. There is no way to do interactive visualization

  if (!fInteractive and fMacroFileNames.empty()) {
    RMGLog::Out(RMGLog::fatal, "Batch mode has been requested but no macro file has been set");
  }

  // configure UI
  std::unique_ptr<G4UIExecutive> session = nullptr;
  if (fInteractive) { session = StartInteractiveSession(); }

  // eventually execute macros
  auto UI = G4UImanager::GetUIpointer();
  for (const auto& macro : fMacroFileNames) {
    RMGLog::Out(RMGLog::summary, "Loading macro file: ", macro);
    UI->ApplyCommand("/control/execute " + macro);
  }

  // if interactive mode is requested, do not quit and start a session
  if (fInteractive) {
    if (!session) session = StartInteractiveSession();
    session->SetPrompt(RMGLog::Colorize<RMGLog::Ansi::unspecified>("remage> ", G4cout, true));
    session->SessionStart();
  }
}

void RMGManager::SetUpDefaultG4RunManager(G4RunManagerType type) {
  RMGLog::Out(RMGLog::debug, "Initializing default run manager");

  // Suppress the Geant4 header:
  // save underlying buffer and set null (only standard output)
  std::streambuf* orig_buf = std::cout.rdbuf();
  std::cout.rdbuf(nullptr);

  fExceptionHandler = new RMGExceptionHandler();
  G4StateManager::GetStateManager()->SetExceptionHandler(fExceptionHandler);

  fG4RunManager = std::unique_ptr<G4RunManager>(G4RunManagerFactory::CreateRunManager(type));
  fG4RunManager->SetVerboseLevel(0);

  // restore buffer
  std::cout.rdbuf(orig_buf);

  // set the appropriate thread init for this run manager type. Use the actuial type to decide, and
  // not the requested type: It is possible to override the resulting run manager type in the environment.
  if (dynamic_cast<G4TaskRunManager*>(fG4RunManager.get()) != nullptr) {
    fG4RunManager->SetUserInitialization(
        new RMGWorkerInitialization<G4UserTaskThreadInitialization>());
  } else if (dynamic_cast<G4MTRunManager*>(fG4RunManager.get()) != nullptr) {
    fG4RunManager->SetUserInitialization(
        new RMGWorkerInitialization<G4UserWorkerThreadInitialization>());
  } else if (!IsExecSequential()) {
    RMGLog::OutDev(RMGLog::fatal, "Unknown type of MT run manager.");
  }
}

void RMGManager::SetUpDefaultUserAction() {
  RMGLog::Out(RMGLog::debug, "Initializing default user action class");
  fUserAction = new RMGUserAction();
}

void RMGManager::SetUpDefaultProcessesList() {
  RMGLog::Out(RMGLog::debug, "Initializing default processes list");
  fPhysicsList = new RMGPhysics();
}

void RMGManager::SetUpDefaultG4VisManager() {
  RMGLog::Out(RMGLog::debug, "Initializing default visualization manager");
  fG4VisManager = std::make_unique<G4VisExecutive>("quiet");
}

void RMGManager::SetUpDefaultDetectorConstruction() {
  RMGLog::Out(RMGLog::debug, "Initializing default (empty) detector");
  fDetectorConstruction = new RMGHardware();
}

G4RunManager* RMGManager::GetG4RunManager() {
  if (!fG4RunManager) this->SetUpDefaultG4RunManager();
  return fG4RunManager.get();
}

G4VisManager* RMGManager::GetG4VisManager() {
  if (!fG4VisManager) this->SetUpDefaultG4VisManager();
  return fG4VisManager.get();
}

RMGHardware* RMGManager::GetDetectorConstruction() {
  if (!fDetectorConstruction) this->SetUpDefaultDetectorConstruction();
  return fDetectorConstruction;
}

G4VUserPhysicsList* RMGManager::GetProcessesList() {
  if (!fPhysicsList) this->SetUpDefaultProcessesList();
  return fPhysicsList;
}

void RMGManager::SetLogLevel(std::string level) {
  try {
    RMGLog::SetLogLevel(RMGTools::ToEnum<RMGLog::LogLevel>(level, "logging level"));
  } catch (const std::bad_cast&) { return; }
}

void RMGManager::SetRandEngine(std::string name) {
  fIsRandControlledAtEngineChange = fIsRandControlled;

  fRandEngineName = name;
  if (!ApplyRandEngineForCurrentThread()) {
    RMGLog::Out(RMGLog::error, "'", fRandEngineName, "' random engine unknown");
  }
  RMGLog::Out(RMGLog::summary, "Using ", CLHEP::HepRandom::getTheEngine()->name(), " random engine");
}

bool RMGManager::ApplyRandEngineForCurrentThread() {
  CLHEP::HepRandomEngine* engine = nullptr;
  if (fRandEngineName == "JamesRandom") {
    engine = new CLHEP::HepJamesRandom;
  } else if (fRandEngineName == "RanLux") {
    // TODO: somehow need to propagate engine->getLuxury() if applying to WTs?
    engine = new CLHEP::RanluxEngine;
  } else if (fRandEngineName == "MTwist") {
    engine = new CLHEP::MTwistEngine;
  } else if (fRandEngineName == "MixMaxRng") {
    engine = new CLHEP::MixMaxRng;
  } else if (!fRandEngineName.empty()) {
    return false;
  }
  if (engine != nullptr) { CLHEP::HepRandom::setTheEngine(engine); }
  return true;
}

void RMGManager::CheckRandEngineMTState() {
  if (fG4RunManager == nullptr || IsExecSequential() || GetRandIsControlled() ||
      fIsRandControlledAtEngineChange || fRandEngineName.empty())
    return;
  RMGLog::Out(RMGLog::warning,
      "Setting a random engine and a seed requires to set a seed before changing the rand engine "
      "in MT mode. Otherwise results are non-deterministic.");
}

void RMGManager::SetRandEngineSeed(long seed) {
  CheckRandEngineMTState();
  if (seed >= std::numeric_limits<long>::max()) {
    RMGLog::Out(RMGLog::error, "Seed ", seed, " is too large. Largest possible seed is ",
        std::numeric_limits<long>::max(), ". Setting seed to 0.");
    CLHEP::HepRandom::setTheSeed(0);
  } else CLHEP::HepRandom::setTheSeed(seed);
  RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", seed);

  fIsRandControlled = true;
}

void RMGManager::SetRandEngineInternalSeed(int index) {
  CheckRandEngineMTState();
  long seeds[2];
  int table_index = index / 2;
  CLHEP::HepRandom::getTheTableSeeds(seeds, table_index);

  int array_index = index % 2;
  CLHEP::HepRandom::setTheSeed(seeds[array_index]);
  RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", seeds[array_index]);

  fIsRandControlled = true;
}

void RMGManager::SetRandSystemEntropySeed() {
  std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
  std::random_device rd; // uses RDRND or /dev/urandom
  auto rand_seed = dist(rd);
  CLHEP::HepRandom::setTheSeed(rand_seed);
  RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", rand_seed);

  // TODO: does this make sense?
  fIsRandControlled = true;
}

void RMGManager::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Manager/",
      "General commands for controlling the application");

  fMessenger->DeclareProperty("Interactive", fInteractive)
      .SetGuidance("Enable interactive mode")
      .SetParameterName("interactive", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("PrintProgressModulo", &RMGManager::SetPrintModulo)
      .SetGuidance("How many processed events before progress information is displayed")
      .SetParameterName("n", false)
      .SetRange("n > 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fLogMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Manager/Logging/",
      "Commands for controlling application logging");

  fLogMessenger->DeclareMethod("LogLevel", &RMGManager::SetLogLevel)
      .SetGuidance("Set verbosity level of application log")
      .SetParameterName("level", false)
      .SetCandidates(RMGTools::GetCandidates<RMGLog::LogLevel>())
      .SetStates(G4State_PreInit, G4State_Idle);

  fRandMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Manager/Randomization/",
      "Commands for controlling randomization settings");

  fRandMessenger->DeclareMethod("RandomEngine", &RMGManager::SetRandEngine)
      .SetGuidance("Select the random engine (CLHEP)")
      .SetParameterName("name", false)
      .SetCandidates("JamesRandom RanLux MTwist MixMaxRng")
      .SetStates(G4State_PreInit, G4State_Idle);

  fRandMessenger->DeclareMethod("Seed", &RMGManager::SetRandEngineSeed)
      .SetGuidance("Select the initial seed for randomization (CLHEP::HepRandom::setTheSeed)")
      .SetParameterName("n", false)
      .SetRange("n >= 0")
      .SetDefaultValue("1")
      .SetStates(G4State_PreInit, G4State_Idle);

  fRandMessenger->DeclareMethod("InternalSeed", &RMGManager::SetRandEngineInternalSeed)
      .SetGuidance("Select the initial seed for randomization by using the internal CLHEP table")
      .SetParameterName("index", false)
      .SetRange("index >= 0 && index < 430")
      .SetStates(G4State_PreInit, G4State_Idle);

  fRandMessenger->DeclareMethod("UseSystemEntropy", &RMGManager::SetRandSystemEntropySeed)
      .SetGuidance("Select a random initial seed from system entropy")
      .SetStates(G4State_PreInit, G4State_Idle);

  fOutputMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Output/",
      "Commands for controlling the simulation output");

  fOutputMessenger->DeclareMethod("FileName", &RMGManager::SetOutputFileName)
      .SetGuidance("Set output file name for object persistency")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fOutputMessenger->DeclareProperty("NtuplePerDetector", fOutputNtuplePerDetector)
      .SetGuidance("Create a ntuple for each sensitive detector to store hits. Otherwise, store "
                   "all hits of one detector type in one ntuple.")
      .SetParameterName("tree_per_det", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fOutputMessenger->DeclareMethod("ActivateOutputScheme", &RMGManager::ActivateOptionalOutputScheme)
      .SetGuidance("Activates the output scheme that had been registered under the given name.")
      .SetParameterName("tree_per_det", false)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

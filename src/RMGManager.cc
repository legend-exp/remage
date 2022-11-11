#include "RMGManager.hh"

#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "G4Threading.hh"
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#endif
#include "G4GenericMessenger.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VUserPhysicsList.hh"
#include "G4VisExecutive.hh"
#include "G4VisManager.hh"
#include "G4Backtrace.hh"
#include "Randomize.hh"

#include "ProjectInfo.hh"
#include "RMGHardware.hh"
#include "RMGPhysics.hh"
#include "RMGTools.hh"
#include "RMGUserAction.hh"

#if RMG_HAS_ROOT
#include "TEnv.h"
#endif

RMGManager* RMGManager::fRMGManager = nullptr;

RMGManager::RMGManager(std::string app_name, int argc, char** argv)
    : fApplicationName(app_name), fArgc(argc), fArgv(argv) {

  if (fRMGManager) RMGLog::Out(RMGLog::fatal, "RMGManager must be singleton!");
  fRMGManager = this;

#if RMG_HAS_ROOT
  // turn off ROOT's stacktrace
  gEnv->SetValue("Root.Stacktrace", 0);
#endif

  // limit Geant4 stacktrace dumping to segfaults
  G4Backtrace::DefaultSignals() = std::set<int>{ SIGSEGV };

  this->DefineCommands();
}

RMGManager::~RMGManager() {
  if (RMGLog::IsOpen()) RMGLog::CloseLog();
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
      RMGLog::OutFormat(RMGLog::detail, "Execution is multi-threaded ({} threads are used)", fNThreads);
    }
  }

  if (!fPhysicsList) this->SetUpDefaultProcessesList();
  if (!fG4VisManager) this->SetUpDefaultG4VisManager();
  fG4VisManager->Initialize();

  std::string _str = "";
  for (const auto& i : fG4VisManager->GetAvailableGraphicsSystems()) {
    _str += i->GetNickname() + " ";
  }
  RMGLog::Out(RMGLog::detail, "Available graphic systems: ", _str);

  if (!fUserAction) {
    RMGLog::Out(RMGLog::debug, "Initializing default user action class");
    fUserAction = new RMGUserAction();
  }

  if (!fDetectorConstruction) this->SetUpDefaultDetectorConstruction();

  fG4RunManager->SetUserInitialization(fDetectorConstruction);
  fG4RunManager->SetUserInitialization(fPhysicsList);
  fG4RunManager->SetUserInitialization(fUserAction);

  if (!fIsRandControlled) {
    std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
    std::random_device rd; // uses RDRND or /dev/urandom
    auto rand_seed = dist(rd);
    G4Random::setTheSeed(rand_seed);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", rand_seed);
  }
}

void RMGManager::Run() {

  if (fBatchMode and fMacroFileNames.empty()) {
    RMGLog::Out(RMGLog::fatal, "Batch mode has been requested but no macro file has been set");
  }

  std::unique_ptr<G4UIExecutive> session = nullptr;
  if (!fBatchMode) {
    RMGLog::Out(RMGLog::summary, "Entering interactive mode");
    auto cval = std::getenv("DISPLAY");
    auto val = cval == nullptr ? std::string("") : std::string(cval);
    if (val.empty()) RMGLog::Out(RMGLog::warning, "DISPLAY not set, forcing G4UI_USE_TCSH=1");
    session = std::make_unique<G4UIExecutive>(fArgc, fArgv, val.empty() ? "tcsh" : "");
  }

  auto UI = G4UImanager::GetUIpointer();
  for (const auto& macro : fMacroFileNames) {
    RMGLog::Out(RMGLog::summary, "Loading macro file: ", macro);
    UI->ApplyCommand("/control/execute " + macro);
  }

  if (!fBatchMode) {
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

  fG4RunManager = std::unique_ptr<G4RunManager>(
      G4RunManagerFactory::CreateRunManager(type));
  fG4RunManager->SetVerboseLevel(0);

  // restore buffer
  std::cout.rdbuf(orig_buf);
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

void RMGManager::SetLogLevelScreen(std::string level) {
  try {
    RMGLog::SetLogLevelScreen(RMGTools::ToEnum<RMGLog::LogLevel>(level, "logging level"));
  } catch (const std::bad_cast&) { return; }
}

void RMGManager::SetLogLevelFile(std::string level) {
  try {
    RMGLog::SetLogLevelFile(RMGTools::ToEnum<RMGLog::LogLevel>(level, "logging level"));
  } catch (const std::bad_cast&) { return; }
}

void RMGManager::SetRandEngine(std::string name) {
  if (name == "JamesRandom") {
    CLHEP::HepRandom::setTheEngine(new CLHEP::HepJamesRandom);
    RMGLog::Out(RMGLog::summary, "Using James random engine");
  } else if (name == "RanLux") {
    CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine);
    RMGLog::Out(RMGLog::summary, "Using RanLux random engine");
  } else if (name == "MTwist") {
    CLHEP::HepRandom::setTheEngine(new CLHEP::MTwistEngine);
    RMGLog::Out(RMGLog::summary, "Using MTwist random engine");
  } else {
    RMGLog::Out(RMGLog::error, "'", name, "' random engine unknown");
  }
}

void RMGManager::SetRandEngineSeed(long seed) {
  if (seed >= std::numeric_limits<long>::max()) {
    RMGLog::Out(RMGLog::error, "Seed ", seed, " is too large. Largest possible seed is ",
        std::numeric_limits<long>::max(), ". Setting seed to 0.");
    CLHEP::HepRandom::setTheSeed(0);
  } else CLHEP::HepRandom::setTheSeed(seed);
  RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", seed);

  fIsRandControlled = true;
}

void RMGManager::SetRandEngineInternalSeed(int index) {
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

  fMessenger->DeclareMethod("Include", &RMGManager::IncludeMacroFile)
      .SetGuidance("Include macro file")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareMethod("PrintProgressModulo", &RMGManager::SetPrintModulo)
      .SetGuidance("How many processed events before progress information is displayed")
      .SetParameterName("n", false)
      .SetRange("n > 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fLogMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Manager/Logging/",
      "Commands for controlling application logging");

  fLogMessenger->DeclareMethod("LogLevelScreen", &RMGManager::SetLogLevelScreen)
      .SetGuidance("Set verbosity level on screen")
      .SetParameterName("level", false)
      .SetCandidates(RMGTools::GetCandidates<RMGLog::LogLevel>())
      .SetStates(G4State_PreInit, G4State_Idle);

  fLogMessenger->DeclareMethod("LogLevelFile", &RMGManager::SetLogLevelFile)
      .SetGuidance("Set verbosity level on file")
      .SetParameterName("level", false)
      .SetCandidates(RMGTools::GetCandidates<RMGLog::LogLevel>())
      .SetStates(G4State_PreInit, G4State_Idle);

  fLogMessenger->DeclareMethod("LogToFile", &RMGManager::SetLogToFileName)
      .SetGuidance("Set filename for dumping application output")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fRandMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Manager/Randomization/",
      "Commands for controlling randomization settings");

  fRandMessenger->DeclareMethod("RandomEngine", &RMGManager::SetRandEngine)
      .SetGuidance("Select the random engine (CLHEP)")
      .SetParameterName("name", false)
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

  fOutputMessenger->DeclareProperty("FileName", fOutputFile)
      .SetGuidance("Set output file name for object persistency")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "RMGManager.hh"

#include <iostream>
#include <string>
#include <vector>
#include <random>

#include "G4Threading.hh"
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#endif
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4VUserPhysicsList.hh"
#include "G4VisManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4GenericMessenger.hh"
#include "Randomize.hh"

#include "RMGProcessesList.hh"
#include "RMGManagementDetectorConstruction.hh"
#include "RMGManagementUserAction.hh"

#include "magic_enum/magic_enum.hpp"

RMGManager* RMGManager::fRMGManager = nullptr;

RMGManager::RMGManager(G4String app_name, int argc, char** argv) :
  fApplicationName(app_name),
  fArgc(argc),
  fArgv(argv),
  fIsRandControlled(false),
  fBatchMode(false),
  fG4RunManager(nullptr),
  fG4VisManager(nullptr),
  fProcessesList(nullptr),
  fManagerDetectorConstruction(nullptr),
  fManagementUserAction(nullptr) {

  if (fRMGManager) RMGLog::Out(RMGLog::fatal, "RMGManager must be singleton!");
  fRMGManager = this;

  // FIXME: I don't like this here
  this->SetupDefaultG4RunManager();

  this->DefineCommands();
}

RMGManager::~RMGManager() {
  if (RMGLog::IsOpen()) RMGLog::CloseLog();
}

void RMGManager::Initialize() {

  RMGLog::Out(RMGLog::detail, "Initializing application");

  if (!fG4RunManager) this->SetupDefaultG4RunManager();
  if (!fProcessesList) this->SetupDefaultRMGProcessesList();
  if (!fG4VisManager) this->SetupDefaultG4VisManager();
  fG4VisManager->Initialize();

  G4String _str = "";
  for (const auto& i : fG4VisManager->GetAvailableGraphicsSystems()) {
    _str += i->GetNickname() + " ";
  }
  RMGLog::Out(RMGLog::detail, "Available graphic systems: ", _str);


  if (!fManagementUserAction) {
    RMGLog::Out(RMGLog::debug, "Initializing default user action class");
    fManagementUserAction = new RMGManagementUserAction();
  }

  if (!fManagerDetectorConstruction) this->SetupDefaultManagementDetectorConstruction();

  fG4RunManager->SetUserInitialization(fManagerDetectorConstruction);
  fG4RunManager->SetUserInitialization(fProcessesList);
  fG4RunManager->SetUserInitialization(fManagementUserAction);

  if (!fIsRandControlled) {
    std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
    std::random_device rd; // uses RDRND or /dev/urandom
    auto rand_seed = dist(rd);
    G4Random::setTheSeed(rand_seed);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", rand_seed);
  }
}

void RMGManager::Run() {

  auto UI = G4UImanager::GetUIpointer();
  for (const auto& macro : fMacroFileNames) {
    RMGLog::Out(RMGLog::summary, "Loading macro file: ", macro);
    UI->ApplyCommand("/control/execute " + macro);
  }

  if (!fBatchMode) {
    RMGLog::Out(RMGLog::summary, "Entering interactive mode");
    auto session = std::make_unique<G4UIExecutive>(fArgc, fArgv);
    session->SetPrompt(RMGLog::Colorize<RMGLog::Ansi::unspecified>("remage> ", G4cout, true));
    session->SessionStart();
  }
}

void RMGManager::SetupDefaultG4RunManager() {
  RMGLog::Out(RMGLog::debug, "Initializing default run manager");

  // Suppress the Geant4 header:
  // save underlying buffer and set null (only standard output)
  std::streambuf* orig_buf = std::cout.rdbuf();
  std::cout.rdbuf(nullptr);

  fG4RunManager = std::unique_ptr<G4RunManager>(
      G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default));
  fG4RunManager->SetVerboseLevel(0);

  // restore buffer
  std::cout.rdbuf(orig_buf);
}

void RMGManager::SetupDefaultRMGProcessesList() {
  RMGLog::Out(RMGLog::debug, "Initializing default processes list");
  fProcessesList = new RMGProcessesList();
}

void RMGManager::SetupDefaultG4VisManager() {
  RMGLog::Out(RMGLog::debug, "Initializing default visualization manager");
  fG4VisManager = std::make_unique<G4VisExecutive>("quiet");
}

void RMGManager::SetupDefaultManagementDetectorConstruction() {
  RMGLog::Out(RMGLog::debug, "Initializing default (empty) detector");
  fManagerDetectorConstruction = new RMGManagementDetectorConstruction();
}

G4RunManager* RMGManager::GetG4RunManager() {
  if (!fG4RunManager) this->SetupDefaultG4RunManager();
  return fG4RunManager.get();
}

G4VisManager* RMGManager::GetG4VisManager() {
  if (!fG4VisManager) this->SetupDefaultG4VisManager();
  return fG4VisManager.get();
}

RMGManagementDetectorConstruction* RMGManager::GetManagementDetectorConstruction() {
  if (!fManagerDetectorConstruction) this->SetupDefaultManagementDetectorConstruction();
  return fManagerDetectorConstruction;
}

G4VUserPhysicsList* RMGManager::GetRMGProcessesList() {
  if (!fProcessesList) this->SetupDefaultRMGProcessesList();
  return fProcessesList;
}

void RMGManager::SetLogLevelScreen(G4String level) {
  auto result = magic_enum::enum_cast<RMGLog::LogLevel>(level);
  if (result.has_value()) RMGLog::SetLogLevelScreen(result.value());
  else RMGLog::Out(RMGLog::error, "Illegal logging level '", level, "'");
}

void RMGManager::SetLogLevelFile(G4String level) {
  auto result = magic_enum::enum_cast<RMGLog::LogLevel>(level);
  if (result.has_value()) RMGLog::SetLogLevelFile(result.value());
  else RMGLog::Out(RMGLog::error, "Illegal logging level '", level, "'");
}

void RMGManager::SetRandEngine(G4String name) {
  if (name == "JamesRandom") {
    CLHEP::HepRandom::setTheEngine(new CLHEP::HepJamesRandom);
    RMGLog::Out(RMGLog::summary, "Using James random engine");
  }
  else if (name == "RanLux") {
    CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine);
    RMGLog::Out(RMGLog::summary, "Using RanLux random engine");
  }
  else if (name == "MTwist") {
    CLHEP::HepRandom::setTheEngine(new CLHEP::MTwistEngine);
    RMGLog::Out(RMGLog::summary, "Using MTwist random engine");
  }
  else {
    RMGLog::Out(RMGLog::error, "'", name, "' random engine unknown");
  }
}

void RMGManager::SetRandEngineSeed(G4long seed) {
  if (seed >= std::numeric_limits<long>::max()) {
    RMGLog::Out(RMGLog::error, "Seed ", seed, " is too large. Largest possible seed is ",
        std::numeric_limits<long>::max(), ". Setting seed to 0.");
    CLHEP::HepRandom::setTheSeed(0);
  }
  else CLHEP::HepRandom::setTheSeed(seed);
  RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", seed);

  fIsRandControlled = true;
}

void RMGManager::SetRandEngineInternalSeed(G4int index) {
  long seeds[2];
  int table_index = index/2;
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

  fIsRandControlled = true;
}

void RMGManager::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Manager/",
      "General commands for controlling the application");

  fMessenger->DeclareMethod("Include", &RMGManager::IncludeMacroFile)
    .SetGuidance("Include macro file")
    .SetParameterName("filename", false)
    .SetStates(G4State_PreInit, G4State_Idle);

  fLogMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Manager/Logging/",
      "Commands for controlling application logging");

  fLogMessenger->DeclareMethod("LogLevelScreen", &RMGManager::SetLogLevelScreen)
    .SetGuidance("Set verbosity level on screen")
    .SetParameterName("level", false)
    .SetStates(G4State_PreInit, G4State_Idle);

  fLogMessenger->DeclareMethod("LogLevelFile", &RMGManager::SetLogLevelFile)
    .SetGuidance("Set verbosity level on file")
    .SetParameterName("level", false)
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
    .SetDefaultValue("1.")
    .SetStates(G4State_PreInit, G4State_Idle);

  fRandMessenger->DeclareMethod("InternalSeed", &RMGManager::SetRandEngineInternalSeed)
    .SetGuidance("Select the initial seed for randomization by using the internal CLHEP table")
    .SetParameterName("index", false)
    .SetRange("index >= 0 && index < 430")
    .SetStates(G4State_PreInit, G4State_Idle);

  fRandMessenger->DeclareMethod("UseSystemEntropy", &RMGManager::SetRandSystemEntropySeed)
    .SetGuidance("Select a random initial seed from system entropy")
    .SetStates(G4State_PreInit, G4State_Idle);

}

// vim: tabstop=2 shiftwidth=2 expandtab

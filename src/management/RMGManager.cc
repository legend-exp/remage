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
#include "G4VisManager.hh"
#include "G4VUserPhysicsList.hh"
#include "G4VisManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "Randomize.hh"

#include "RMGProcessesList.hh"
#include "RMGManagementDetectorConstruction.hh"
#include "RMGManagementUserAction.hh"
#include "RMGLog.hh"
#include "RMGManagerMessenger.hh"

RMGManager* RMGManager::fRMGManager = nullptr;

RMGManager::RMGManager(G4String app_name, int argc, char** argv) :
  fApplicationName(app_name),
  fArgc(argc),
  fArgv(argv),
  fMacroFileName(""),
  fControlledRandomization(false),
  fSessionType(RMGManager::kDefault),
  fG4RunManager(nullptr),
  fG4VisManager(nullptr),
  fProcessesList(nullptr),
  fManagerDetectorConstruction(nullptr),
  fManagementUserAction(nullptr) {

  if (fRMGManager) RMGLog::Out(RMGLog::fatal, "RMGManager must be singleton!");
  fRMGManager = this;

  // FIXME: I don't like this here
  this->SetupDefaultG4RunManager();
  fG4Messenger = std::make_unique<RMGManagerMessenger>(this);
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

  if (!fManagementUserAction) {
    RMGLog::Out(RMGLog::debug, "Initializing default user action class");
    fManagementUserAction = new RMGManagementUserAction();
  }

  if (!fManagerDetectorConstruction) this->SetupDefaultManagementDetectorConstruction();

  fG4RunManager->SetUserInitialization(fManagerDetectorConstruction);
  fG4RunManager->SetUserInitialization(fProcessesList);
  fG4RunManager->SetUserInitialization(fManagementUserAction);

  if (!fControlledRandomization) {
    std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
    std::random_device rd; // uses RDRND or /dev/urandom
    auto rand_seed = dist(rd);
    G4Random::setTheSeed(rand_seed);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", rand_seed);
  }

  fG4RunManager->Initialize();
}

void RMGManager::Run() {
  if (fMacroFileName.empty()) {
    RMGLog::Out(RMGLog::summary, "Entering interactive mode");
    auto session = std::make_unique<G4UIExecutive>(fArgc, fArgv);
    session->SetPrompt("remage>");
    session->SessionStart();
  }
  else {
    RMGLog::Out(RMGLog::summary, "Entering batch mode");
    RMGLog::Out(RMGLog::summary, "Executing script file from command line: ", fMacroFileName);
    auto UI = G4UImanager::GetUIpointer();
    UI->ApplyCommand("/control/execute " + fMacroFileName);
  }
}

void RMGManager::SetupDefaultG4RunManager() {
  // Suppress the Geant4 header:
  // save underlying buffer and set null (only standard output)
  std::streambuf* orig_buf = std::cout.rdbuf();
  std::cout.rdbuf(nullptr);

  RMGLog::Out(RMGLog::debug, "Initializing default run manager");
  fG4RunManager = std::unique_ptr<G4RunManager>(G4RunManagerFactory::CreateRunManager());
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

void RMGManager::PrintUsage() {
  std::cout << fApplicationName << ": USAGE" << std::endl;
}

// vim: tabstop=2 shiftwidth=2 expandtab

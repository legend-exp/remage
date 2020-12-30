#include "RMGManager.hh"

#include <iostream>
#include <getopt.h>
#include <string>
#include <vector>

#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisManager.hh"
#include "G4VUserPhysicsList.hh"
#include "G4UImanager.hh"
#include "G4UIsession.hh"
#include "G4UIterminal.hh"
#include "G4VisManager.hh"
#include "G4UIExecutive.hh"
#include "G4UItcsh.hh"
#include "G4VisExecutive.hh"

#include "RMGProcessesList.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGManagerDetectorConstruction.hh"
#include "RMGManagementEventAction.hh"
#include "RMGManagementRunAction.hh"
#include "RMGManagementSteppingAction.hh"
#include "RMGManagementTrackingAction.hh"
#include "RMGManagementStackingAction.hh"
#include "RMGLog.hh"
#include "RMGManagerMessenger.hh"

RMGManager* RMGManager::fRMGManager = nullptr;

RMGManager::RMGManager(G4String app_name) :
  fApplicationName(app_name),
  fMacroFileName("") {

  if (fRMGManager) RMGLog::Out(RMGLog::fatal, "RMGManager must be singleton!");
  fRMGManager = this;
  fG4Messenger = new RMGManagerMessenger(this);
}

RMGManager::~RMGManager() {
  delete fG4Messenger;
  if (RMGLog::IsOpen()) RMGLog::CloseLog();
}

void RMGManager::Initialize() {
  if (!fG4RunManager) fG4RunManager = std::unique_ptr<G4RunManager>(G4RunManagerFactory::CreateRunManager());
  if (!fG4VisManager) fG4VisManager = std::unique_ptr<G4VisManager>(new G4VisExecutive());
  if (!fProcessesList) fProcessesList = new RMGProcessesList();
  if (!fGeneratorPrimary) fGeneratorPrimary = new RMGGeneratorPrimary();
  if (!fManagementRunAction) fManagementRunAction = new RMGManagementRunAction();
  if (!fManagementEventAction) fManagementEventAction = new RMGManagementEventAction();
  if (!fManagementStackingAction) fManagementStackingAction = new RMGManagementStackingAction(fManagementEventAction);
  if (!fManagementSteppingAction) fManagementSteppingAction = new RMGManagementSteppingAction(fManagementEventAction);
  if (!fManagementTrackingAction) fManagementTrackingAction = new RMGManagementTrackingAction(fManagementEventAction);
  // if (!fManagerDetectorConstruction) fManagerDetectorConstruction = new RMGManagerDetectorConstruction();
}

void RMGManager::Run() {
    if (fMacroFileName.empty()) {
    // MGLog(routine) << "Entering interactive mode." << endlog;
    // G4UIExecutive *session = new G4UIExecutive(argc,argv);
    // session->SessionStart();
    // delete session;
  }
  else {
    // MGLog(routine) << "Entering batch mode..." << endlog;
    // MGLog(routine) << "Executing script file from command line: " << *(args.end()-1) << endlog;
    auto UI = G4UImanager::GetUIpointer();
    UI->ApplyCommand("/control/execute " + fMacroFileName);
  }
}

G4bool RMGManager::ParseCommandLineArgs(int argc, char** argv) {

    const char* const short_opts = ":h";
    const option long_opts[] = {
        { "help",  no_argument, nullptr, 'h' },
        { nullptr, no_argument, nullptr, 0   }
    };

    int opt = 0;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'h': // -h or --help
            case '?': // Unrecognized option
            default:
                this->PrintUsage();
                return false;
        }
    }

    // extra arguments
    std::vector<std::string> args;
    for(; optind < argc; optind++) {
        args.emplace_back(argv[optind]);
    }

    if (!args.empty()) fMacroFileName = args[0];

    return true;
}

void RMGManager::PrintUsage() {
  std::cout << fApplicationName << ": USAGE" << std::endl;
}

// vim: tabstop=2 shiftwidth=2 expandtab

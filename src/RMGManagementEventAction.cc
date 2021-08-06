#include "RMGManagementEventAction.hh"

#include <sstream>
#include <chrono>

#include "G4RunManager.hh"
#include "RMGRun.hh"

#include "G4GenericMessenger.hh"
#include "RMGVOutputManager.hh"
#include "RMGManager.hh"
#include "RMGManagementRunAction.hh"
#include "RMGManagementUserAction.hh"
#include "RMGLog.hh"

RMGManagementEventAction::RMGManagementEventAction() {
  this->DefineCommands();
}

RMGManagementEventAction::~RMGManagementEventAction() {
  // if (fOutputManager) {
  //   fOutputManager->CloseFile();
  //   delete fOutputManager;
  // }
}

void RMGManagementEventAction::BeginOfEventAction(const G4Event* event) {

  auto print_modulo = G4RunManager::GetRunManager()->GetPrintProgress();
  if ((event->GetEventID()+1) % print_modulo == 0) {

    auto current_run = dynamic_cast<const RMGRun*>(G4RunManager::GetRunManager()->GetCurrentRun());
    auto start_time = current_run->GetStartTime();
    auto tot_events = current_run->GetNumberOfEventToBeProcessed();

    auto time_now = std::chrono::system_clock::now();
    auto t_sec = std::chrono::duration_cast<std::chrono::seconds>(time_now - start_time).count();
    auto t_days = (t_sec - (t_sec % 86400)) / 86400;
    t_sec -= 86400 * t_sec;
    auto t_hours = (t_sec - (t_sec % 3600)) / 3600;
    t_sec -= 3600 * t_hours;
    auto t_minutes = (t_sec - (t_sec % 60)) / 60;
    t_sec -= 60 * t_minutes;

    // FIXME: how to cast the percentage to int?
    RMGLog::OutFormat(RMGLog::summary, "Processing event nr. {:d} ({}%), at {:d} days, {:d} hours, {:d} minutes and {:d} seconds",
        event->GetEventID(), (event->GetEventID()+1)*100./tot_events, t_days, t_hours, t_minutes, t_sec);
  }

  // if (fOutputManager) fOutputManager->BeginOfEventAction(event);
}

void RMGManagementEventAction::EndOfEventAction(const G4Event*) {

  // if (fOutputManager) fOutputManager->EndOfEventAction(event);
}

void RMGManagementEventAction::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Output/",
      "Commands for controlling the event actions");

  // fMessenger->DeclareMethod("FileName"
}

// vim: tabstop=2 shiftwidth=2 expandtab

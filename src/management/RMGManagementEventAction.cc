#include "RMGManagementEventAction.hh"

#include <sstream>
#include <chrono>

#include "G4RunManager.hh"
#include "G4Run.hh"

#include "RMGManagementEventActionMessenger.hh"
#include "RMGVOutputManager.hh"
#include "RMGManager.hh"
#include "RMGManagementRunAction.hh"
#include "RMGLog.hh"

RMGManagementEventAction::RMGManagementEventAction() :
  fReportingFrequency(100000),
  fWriteOutFrequency(0),
  fWriteOutFileDuringRun(false) {

  fG4Messenger = new RMGManagementEventActionMessenger(this);
}


RMGManagementEventAction::~RMGManagementEventAction() {
  if (fOutputManager) {
    fOutputManager->CloseFile();
    delete fOutputManager;
  }
  delete fG4Messenger;
}

void RMGManagementEventAction::BeginOfEventAction(const G4Event* event) {

  if ((event->GetEventID()+1) % fReportingFrequency == 0) {

    auto start_time = RMGManager::GetRMGManager()->GetRMGRunAction()->GetStartTime();
    auto tot_events = RMGManager::GetRMGManager()->GetG4RunManager()->GetCurrentRun()->GetNumberOfEventToBeProcessed();

    auto time_now = std::chrono::system_clock::now();
    auto t_sec = std::chrono::duration_cast<std::chrono::seconds>(time_now - start_time).count();
    auto t_days = (t_sec - (t_sec % 86400)) / 86400;
    t_sec -= 86400 * t_sec;
    auto t_hours = (t_sec - (t_sec % 3600)) / 3600;
    t_sec -= 3600 * t_hours;
    auto t_minutes = (t_sec - (t_sec % 60)) / 60;
    t_sec -= 60 * t_minutes;

    RMGLog::OutFormat(RMGLog::summary, "Processing event nr. %i (%i%%), at %i days, %i hours, %i minutes and %i seconds",
        event->GetEventID(), (event->GetEventID()+1.)/tot_events, t_days, t_hours, t_minutes, t_sec);
  }

  if (fOutputManager) fOutputManager->BeginOfEventAction(event);
}

void RMGManagementEventAction::EndOfEventAction(const G4Event *event) {

  if (fWriteOutFileDuringRun and fOutputManager) {
    if (fWriteOutFrequency <= 0) fWriteOutFrequency = fReportingFrequency;
    if (event->GetEventID() % fWriteOutFrequency == 0) fOutputManager->WriteFile();
  }
  if (fOutputManager) fOutputManager->EndOfEventAction(event);
}

// vim: tabstop=2 shiftwidth=2 expandtab

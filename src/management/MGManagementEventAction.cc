#include "MGManagementEventAction.hh"

#include <sstream>
// #include <ctime>

#include "MGManagementEventActionMessenger.hh"
#include "MGVOutputManager.hh"
#include "MGManager.hh"
#include "MGManagementRunAction.hh"
#include "MGLog.hh"


MGManagementEventAction::MGManagementEventAction() :
  fReportingFrequency(100000),
  fWriteOutFrequency(0),
  fWriteOutFileDuringRun(false) {

  fG4Messenger = new MGManagementEventActionMessenger(this);
}


MGManagementEventAction::~MGManagementEventAction() {
  if (fOutputManager) {
    fOutputManager->CloseFile();
    delete fOutputManager;
  }
  delete fG4Messenger;
}

void MGManagementEventAction::BeginOfEventAction(const G4Event *event) {

  if (event->GetEventID() % fReportingFrequency == 0) {
      auto manager = MGManager::GetMGManager();
      time_t endTime = time(nullptr);
      G4double timeDifference = difftime(endTime, manager->GetMGRunAction()->GetStartTime());
      tm *z = localtime(&endTime);

      std::stringstream ss;
      ss << " Processing Event # " << event->GetEventID()
        << " at " << timeDifference << " seconds (real time), at the moment it is: " << z->tm_mon+1 << "."
        << z->tm_mday << "." << z->tm_year+1900 << ", " << z->tm_hour << ":" << z->tm_min << ":" << z->tm_sec;
      MGLog::OutSummary(ss.str());
    }

  if (fOutputManager) fOutputManager->BeginOfEventAction(event);
}

void MGManagementEventAction::EndOfEventAction(const G4Event *event) {

  if (fWriteOutFileDuringRun and fOutputManager) {
    if (fWriteOutFrequency <= 0) fWriteOutFrequency = fReportingFrequency;
    if (event->GetEventID() % fWriteOutFrequency == 0) fOutputManager->WriteFile();
  }
  if (fOutputManager) fOutputManager->EndOfEventAction(event);
}

// vim: tabstop=2 shiftwidth=2 expandtab

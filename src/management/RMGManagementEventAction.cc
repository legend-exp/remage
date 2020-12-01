#include "RMGManagementEventAction.hh"

#include <sstream>
// #include <ctime>

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

void RMGManagementEventAction::BeginOfEventAction(const G4Event *event) {

  if (event->GetEventID() % fReportingFrequency == 0) {
      auto manager = RMGManager::GetRMGManager();
      time_t endTime = time(nullptr);
      G4double timeDifference = difftime(endTime, manager->GetRMGRunAction()->GetStartTime());
      tm *z = localtime(&endTime);

      RMGLog::Out(RMGLog::summary, " Processing Event ", event->GetEventID(), " at ", timeDifference,
          " seconds (real time), at the moment it is: ", z->tm_mon+1, ".", z->tm_mday, ".",
          z->tm_year+1900, ", ", z->tm_hour, ":", z->tm_min, ":", z->tm_sec);
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

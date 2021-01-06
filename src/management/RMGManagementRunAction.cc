#include "RMGManagementRunAction.hh"

#include <limits>
#include <ctime>
#include <cstdlib>

#include "G4Run.hh"

#include "RMGRun.hh"
#include "RMGLog.hh"
#include "RMGVOutputManager.hh"
#include "RMGManager.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGVGenerator.hh"
#include "RMGManagementEventAction.hh"
#include "RMGTools.hh"

G4Run* RMGManagementRunAction::GenerateRun() {
  fRMGRun = new RMGRun();
  return fRMGRun;
}

RMGManagementRunAction::RMGManagementRunAction(RMGGeneratorPrimary* gene) {
  fRMGGeneratorPrimary = gene;
}

void RMGManagementRunAction::BeginOfRunAction(const G4Run*) {

  RMGLog::Out(RMGLog::detail, "Performing RMG beginning of run actions");

  if (fRMGGeneratorPrimary) {
    fRMGGeneratorPrimary->GetRMGGenerator()->BeginOfRunAction(fRMGRun);
  }

  // if (manager->GetRMGEventAction()->GetOutputManager()) {
  //   manager->GetRMGEventAction()->GetOutputManager()->BeginOfRunAction();
  // }
  // else RMGLog::Out(RMGLog::warning, "No Output specified!");

  if (this->IsMaster()) {
    // save start time for future
    fRMGRun->SetStartTime(std::chrono::system_clock::now());
    auto tt = RMGTools::ToUTCTime(fRMGRun->GetStartTime());

    RMGLog::OutFormat(RMGLog::summary, "Starting run nr. %i. Current time is %i/%i/%i %i:%i:%i (UTC)",
        fRMGRun->GetRunID(), tt.tm_mday, tt.tm_mon+1, tt.tm_year+1900, tt.tm_hour, tt.tm_min, tt.tm_sec);
    RMGLog::OutFormat(RMGLog::summary, "Number of events to be processed: %i (%g)",
        fRMGRun->GetNumberOfEventToBeProcessed(), fRMGRun->GetNumberOfEventToBeProcessed());
  }
}

void RMGManagementRunAction::EndOfRunAction(const G4Run*) {

  if (fRMGGeneratorPrimary) {
    fRMGGeneratorPrimary->GetRMGGenerator()->EndOfRunAction(fRMGRun);
  }
  // if (manager->GetRMGEventAction()->GetOutputManager()) {
  //   manager->GetRMGEventAction()->GetOutputManager()->EndOfRunAction();
  // }

  if (this->IsMaster()) {
    auto time_now = std::chrono::system_clock::now();
    auto tt = RMGTools::ToUTCTime(time_now);
    RMGLog::OutFormat(RMGLog::summary, "Run nr. %i completed. %i (%g) events simulated. Current time is %i/%i/%i %i:%i:%i (UTC)",
        fRMGRun->GetRunID(), fRMGRun->GetNumberOfEventToBeProcessed(), fRMGRun->GetNumberOfEventToBeProcessed(),
        tt.tm_mday, tt.tm_mon+1, tt.tm_year+1900, tt.tm_hour, tt.tm_min, tt.tm_sec);

      auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(time_now - fRMGRun->GetStartTime()).count();
      auto t_sec = total_sec;
      auto t_days = (t_sec - (t_sec % 86400)) / 86400;
      t_sec -= 86400 * t_sec;
      auto t_hours = (t_sec - (t_sec % 3600)) / 3600;
      t_sec -= 3600 * t_hours;
      auto t_minutes = (t_sec - (t_sec % 60)) / 60;
      t_sec -= 60 * t_minutes;

      RMGLog::OutFormat(RMGLog::summary, "Stats: run time was %i days, %i hours, %i minutes and %i seconds",
          t_days, t_hours, t_minutes, t_sec);

      RMGLog::OutFormat(RMGLog::summary, "Stats: average event processing time was %g seconds/event",
          total_sec*1./fRMGRun->GetNumberOfEvent());
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

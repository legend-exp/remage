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
  fRun = new RMGRun();
  return fRun;
}

RMGManagementRunAction::RMGManagementRunAction() {}

void RMGManagementRunAction::BeginOfRunAction(const G4Run*) {

  auto manager = G4RunManager::GetRunManager();
  RMGLog::Out(RMGLog::detail, "Performing RMG beginning of run actions");

  auto primary_generator = manager->GetUserPrimaryGeneratorAction();
  if (primary_generator) {
    dynamic_cast<const RMGGeneratorPrimary*>(primary_generator)->GetRMGGenerator()->BeginOfRunAction(fRun);
  }
  else RMGLog::Out(RMGLog::fatal, "No generator specified!");

  // if (manager->GetRMGEventAction()->GetOutputManager()) {
  //   manager->GetRMGEventAction()->GetOutputManager()->BeginOfRunAction();
  // }
  // else RMGLog::Out(RMGLog::warning, "No Output specified!");

  if (this->IsMaster()) {
    // save start time for future
    fRun->SetStartTime(std::chrono::system_clock::now());
    auto tt = RMGTools::ToUTCTime(fRun->GetStartTime());

    RMGLog::OutFormat(RMGLog::summary, "Starting run nr. %i. Current time is %i/%i/%i %i:%i:%i (UTC)",
        fRun->GetRunID(), tt.tm_mday, tt.tm_mon+1, tt.tm_year+1900, tt.tm_hour, tt.tm_min, tt.tm_sec);
    RMGLog::OutFormat(RMGLog::summary, "Number of events to be processed: %i (%g)",
        fRun->GetNumberOfEventToBeProcessed(), fRun->GetNumberOfEventToBeProcessed());
  }
}

void RMGManagementRunAction::EndOfRunAction(const G4Run*) {

  auto primary_generator = G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction();
  dynamic_cast<const RMGGeneratorPrimary*>(primary_generator)->GetRMGGenerator()->BeginOfRunAction(fRun);
  // if (manager->GetRMGEventAction()->GetOutputManager()) {
  //   manager->GetRMGEventAction()->GetOutputManager()->EndOfRunAction();
  // }

  if (this->IsMaster()) {
    auto time_now = std::chrono::system_clock::now();
    auto tt = RMGTools::ToUTCTime(time_now);
    RMGLog::OutFormat(RMGLog::summary, "Run nr. %i completed. %i (%g) events simulated. Current time is %i/%i/%i %i:%i:%i (UTC)",
        fRun->GetRunID(), fRun->GetNumberOfEventToBeProcessed(), fRun->GetNumberOfEventToBeProcessed(),
        tt.tm_mday, tt.tm_mon+1, tt.tm_year+1900, tt.tm_hour, tt.tm_min, tt.tm_sec);

      auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(time_now - fRun->GetStartTime()).count();
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
          total_sec*1./fRun->GetNumberOfEvent());
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

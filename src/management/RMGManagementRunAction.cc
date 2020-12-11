#include "RMGManagementRunAction.hh"

#include <random>
#include <limits>
#include <ctime>
#include <cstdlib>

#include "Randomize.hh"
#include "G4Run.hh"

#include "RMGLog.hh"
#include "RMGVOutputManager.hh"
#include "RMGManager.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGVGenerator.hh"
#include "RMGManagementEventAction.hh"
#include "RMGTools.hh"

RMGManagementRunAction::RMGManagementRunAction() : fControlledRandomization(false) {}

void RMGManagementRunAction::BeginOfRunAction(const G4Run *run) {

  auto manager = RMGManager::GetRMGManager();
  RMGLog::Out(RMGLog::detail, "Performing RMG beginning of run actions");
  if (manager->GetRMGGeneratorPrimary()) {
    manager->GetRMGGeneratorPrimary()->GetRMGGenerator()->BeginOfRunAction(run);
  }
  else RMGLog::Out(RMGLog::fatal, "No generator specified!");

  if (manager->GetRMGEventAction()->GetOutputManager()) {
    manager->GetRMGEventAction()->GetOutputManager()->BeginOfRunAction(); 
  }
  else RMGLog::Out(RMGLog::warning, "No Output specified!");

  if (!fControlledRandomization) {
    std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
    std::random_device rd; // uses RDRND or /dev/urandom
    auto rand_seed = dist(rd);
    CLHEP::HepRandom::setTheSeed(rand_seed);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", rand_seed);
  }

  // save start time for future
  fStartTime = std::chrono::system_clock::now();
  auto tt = RMGTools::ToUTCTime(fStartTime);

  RMGLog::OutFormat(RMGLog::summary, "Starting run nr. %i. Current time is %i/%i/%i %i:%i:%i (UTC)",
      run->GetRunID(), tt.tm_mday, tt.tm_mon+1, tt.tm_year+1900, tt.tm_hour, tt.tm_min, tt.tm_sec);
  RMGLog::OutFormat(RMGLog::summary, "Number of events to be processed: %i (%g)",
      run->GetNumberOfEventToBeProcessed(), run->GetNumberOfEventToBeProcessed());
}

void RMGManagementRunAction::EndOfRunAction(const G4Run* run) {

  auto manager = RMGManager::GetRMGManager();
  manager->GetRMGGeneratorPrimary()->GetRMGGenerator()->EndOfRunAction(run);
  if (manager->GetRMGEventAction()->GetOutputManager()) {
    manager->GetRMGEventAction()->GetOutputManager()->EndOfRunAction();
  }

  auto time_now = std::chrono::system_clock::now();
  auto tt = RMGTools::ToUTCTime(time_now);
  RMGLog::OutFormat(RMGLog::summary, "Run nr. %i completed. %i (%g) events simulated. Current time is %i/%i/%i %i:%i:%i (UTC)",
      run->GetRunID(), run->GetNumberOfEventToBeProcessed(), run->GetNumberOfEventToBeProcessed(),
      tt.tm_mday, tt.tm_mon+1, tt.tm_year+1900, tt.tm_hour, tt.tm_min, tt.tm_sec);

    auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(time_now - fStartTime).count();
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
        total_sec*1./run->GetNumberOfEvent());
}

// vim: tabstop=2 shiftwidth=2 expandtab

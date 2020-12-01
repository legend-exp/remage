#include "RMGManagementRunAction.hh"

#include "Randomize.hh"
// #include "G4Run.hh"

#include "RMGLog.hh"
#include "RMGVOutputManager.hh"
#include "RMGManager.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGVGenerator.hh"
#include "RMGManagementEventAction.hh"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

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

  struct timeval tt;
  pid_t pid = getpid();
  gettimeofday(&tt, nullptr);
  G4long time_seed = tt.tv_sec * 1000 + tt.tv_usec/1000;
  time_seed += pid*3137;
  time_seed = std::abs(time_seed);

  if (!fControlledRandomization) {
    CLHEP::HepRandom::setTheSeed(time_seed);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed  set to: ", time_seed);
  }

  fStartTime = tt.tv_sec;
  struct tm *timeInfo;
  timeInfo = gmtime(&fStartTime);
  // RMGLog::OutSummary("Run #" << run->GetRunID() << " started at GMT " << asctime(timeInfo));
}

void RMGManagementRunAction::EndOfRunAction(const G4Run *run) {

  auto manager = RMGManager::GetRMGManager();
  RMGLog::Out(RMGLog::detail, "Performing RMG end of run actions.");
  manager->GetRMGGeneratorPrimary()->GetRMGGenerator()->EndOfRunAction(run);
  if (manager->GetRMGEventAction()->GetOutputManager()) {
    manager->GetRMGEventAction()->GetOutputManager()->EndOfRunAction();
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

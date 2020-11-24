#include "MGManagementRunAction.hh"

#include "Randomize.hh"
// #include "G4Run.hh"

#include "MGLog.hh"
#include "MGVOutputManager.hh"
#include "MGManager.hh"
#include "MGGeneratorPrimary.hh"
#include "MGVGenerator.hh"
#include "MGManagementEventAction.hh"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

MGManagementRunAction::MGManagementRunAction() : fControlledRandomization(false) {}

void MGManagementRunAction::BeginOfRunAction(const G4Run *run) {

  auto manager = MGManager::GetMGManager();
  MGLog::OutDetail("Performing MG beginning of run actions");
  if (manager->GetMGGeneratorPrimary()) {
    manager->GetMGGeneratorPrimary()->GetMGGenerator()->BeginOfRunAction(run);
  }
  else MGLog::OutFatal("No generator specified!");

  if (manager->GetMGEventAction()->GetOutputManager()) {
    manager->GetMGEventAction()->GetOutputManager()->BeginOfRunAction(); 
  }
  else MGLog::OutWarning("No Output specified!");

  struct timeval tt;
  pid_t pid = getpid();
  gettimeofday(&tt, nullptr);
  G4long time_seed = tt.tv_sec * 1000 + tt.tv_usec/1000;
  time_seed += pid*3137;
  time_seed = std::abs(time_seed);

  if (!fControlledRandomization) {
    CLHEP::HepRandom::setTheSeed(time_seed);
    MGLog::OutSummary("CLHEP::HepRandom seed  set to: " + std::to_string(time_seed));
  }

  fStartTime = tt.tv_sec;
  struct tm *timeInfo;
  timeInfo = gmtime(&fStartTime);
  // MGLog::OutSummary("Run #" << run->GetRunID() << " started at GMT " << asctime(timeInfo));
}

void MGManagementRunAction::EndOfRunAction(const G4Run *run) {

  auto manager = MGManager::GetMGManager();
  MGLog::OutDetail("Performing MG end of run actions.");
  manager->GetMGGeneratorPrimary()->GetMGGenerator()->EndOfRunAction(run);
  if (manager->GetMGEventAction()->GetOutputManager()) {
    manager->GetMGEventAction()->GetOutputManager()->EndOfRunAction();
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

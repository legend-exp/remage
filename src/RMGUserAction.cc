#include "RMGUserAction.hh"

#include "RMGEventAction.hh"
#include "RMGManager.hh"
#include "RMGMasterGenerator.hh"
#include "RMGRunAction.hh"
#include "RMGStackingAction.hh"
#include "RMGSteppingAction.hh"
#include "RMGTrackingAction.hh"

void RMGUserAction::BuildForMaster() const {
  // the master thread does not simulate anything.
  // initialize the master generator also on the master thread, to make sure that particle source
  // commands are available early on (following a note in G4GeneralParticleSourceMessenger.hh).
  auto generator_primary = new RMGMasterGenerator();
  this->SetUserAction(
      new RMGRunAction(generator_primary, RMGManager::GetRMGManager()->IsPersistencyEnabled()));
}

void RMGUserAction::Build() const {

  auto generator_primary = new RMGMasterGenerator();
  auto run_action =
      new RMGRunAction(generator_primary, RMGManager::GetRMGManager()->IsPersistencyEnabled());
  auto event_action = new RMGEventAction(run_action);

  this->SetUserAction(generator_primary);
  this->SetUserAction(event_action);
  this->SetUserAction(run_action);
  this->SetUserAction(new RMGStackingAction(event_action));
  this->SetUserAction(new RMGSteppingAction(event_action));
  this->SetUserAction(new RMGTrackingAction(event_action));
}

// vim: tabstop=2 shiftwidth=2 expandtab

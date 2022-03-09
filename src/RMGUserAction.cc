#include "RMGUserAction.hh"

#include "RMGManager.hh"
#include "RMGMasterGenerator.hh"
#include "RMGRunAction.hh"
#include "RMGEventAction.hh"
#include "RMGStackingAction.hh"
#include "RMGTrackingAction.hh"
#include "RMGSteppingAction.hh"

void RMGUserAction::BuildForMaster() const {

  // the master thread does not simulate anything
  this->SetUserAction(new RMGRunAction(RMGManager::GetRMGManager()->IsPersistencyEnabled()));
}

void RMGUserAction::Build() const {

  auto generator_primary = new RMGMasterGenerator();
  auto run_action = new RMGRunAction(generator_primary, RMGManager::GetRMGManager()->IsPersistencyEnabled());
  auto event_action = new RMGEventAction(run_action);

  this->SetUserAction(generator_primary);
  this->SetUserAction(event_action);
  this->SetUserAction(run_action);
  this->SetUserAction(new RMGStackingAction(event_action));
  this->SetUserAction(new RMGSteppingAction(event_action));
  this->SetUserAction(new RMGTrackingAction(event_action));
}

// vim: tabstop=2 shiftwidth=2 expandtab

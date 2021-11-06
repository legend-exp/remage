#include "RMGUserAction.hh"

#include "RMGMasterGenerator.hh"
#include "RMGRunAction.hh"
#include "RMGEventAction.hh"
#include "RMGStackingAction.hh"
#include "RMGTrackingAction.hh"
#include "RMGSteppingAction.hh"

void RMGUserAction::BuildForMaster() const {

  // the master thread does not simulate anything, no primary generator is needed
  this->SetUserAction(new RMGRunAction());
}

void RMGUserAction::Build() const {

  auto generator_primary = new RMGMasterGenerator();
  this->SetUserAction(generator_primary);
  this->SetUserAction(new RMGRunAction(generator_primary));
  auto event_action = new RMGEventAction();
  this->SetUserAction(event_action);
  this->SetUserAction(new RMGStackingAction(event_action));
  this->SetUserAction(new RMGSteppingAction(event_action));
  this->SetUserAction(new RMGTrackingAction(event_action));
}

// vim: tabstop=2 shiftwidth=2 expandtab

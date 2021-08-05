#include "RMGManagementUserAction.hh"

#include "RMGGeneratorPrimary.hh"
#include "RMGManagementRunAction.hh"
#include "RMGManagementEventAction.hh"
#include "RMGManagementStackingAction.hh"
#include "RMGManagementTrackingAction.hh"
#include "RMGManagementSteppingAction.hh"

void RMGManagementUserAction::BuildForMaster() const {

  // the master thread does not simulate anything, no primary generator is needed
  this->SetUserAction(new RMGManagementRunAction());
}

void RMGManagementUserAction::Build() const {

  auto generator_primary = new RMGGeneratorPrimary();
  this->SetUserAction(generator_primary);
  this->SetUserAction(new RMGManagementRunAction(generator_primary));
  auto event_action = new RMGManagementEventAction();
  this->SetUserAction(event_action);
  this->SetUserAction(new RMGManagementStackingAction(event_action));
  this->SetUserAction(new RMGManagementSteppingAction(event_action));
  this->SetUserAction(new RMGManagementTrackingAction(event_action));
}

// vim: tabstop=2 shiftwidth=2 expandtab

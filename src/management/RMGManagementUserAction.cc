#include "RMGManagementUserAction.hh"

#include "RMGGeneratorPrimary.hh"
#include "RMGManagementRunAction.hh"
#include "RMGManagementEventAction.hh"
#include "RMGManagementStackingAction.hh"
#include "RMGManagementTrackingAction.hh"
#include "RMGManagementSteppingAction.hh"

void RMGManagementUserAction::BuildForMaster() const {

  this->SetUserAction(new RMGManagementRunAction());
}

void RMGManagementUserAction::Build() const {

  this->SetUserAction(new RMGGeneratorPrimary());
  this->SetUserAction(new RMGManagementRunAction());
  auto event_action = new RMGManagementEventAction();
  this->SetUserAction(event_action);
  this->SetUserAction(new RMGManagementStackingAction(event_action));
  this->SetUserAction(new RMGManagementSteppingAction(event_action));
  this->SetUserAction(new RMGManagementTrackingAction(event_action));
}

// vim: tabstop=2 shiftwidth=2 expandtab

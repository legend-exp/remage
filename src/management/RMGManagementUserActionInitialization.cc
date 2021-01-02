#include "RMGManagementUserActionInitialization.hh"

#include "RMGGeneratorPrimary.hh"
#include "RMGManagementRunAction.hh"
#include "RMGManagementEventAction.hh"
#include "RMGManagementStackingAction.hh"
#include "RMGManagementTrackingAction.hh"
#include "RMGManagementSteppingAction.hh"

void RMGManagementUserActionInitialization::BuildForMaster() const {

  // if (!fManagementRunAction) fManagementRunAction = new RMGManagementRunAction();

  this->SetUserAction(new RMGManagementRunAction());
}

void RMGManagementUserActionInitialization::Build() const {

  // if (!fGeneratorPrimary) fGeneratorPrimary = new RMGGeneratorPrimary();
  // if (!fManagementEventAction) fManagementEventAction = new RMGManagementEventAction();
  // if (!fManagementStackingAction) fManagementStackingAction = new RMGManagementStackingAction(fManagementEventAction);
  // if (!fManagementSteppingAction) fManagementSteppingAction = new RMGManagementSteppingAction(fManagementEventAction);
  // if (!fManagementTrackingAction) fManagementTrackingAction = new RMGManagementTrackingAction(fManagementEventAction);

  this->SetUserAction(new RMGGeneratorPrimary());
  this->SetUserAction(new RMGManagementRunAction());
  auto event_action = new RMGManagementEventAction();
  this->SetUserAction(event_action);
  this->SetUserAction(new RMGManagementStackingAction(event_action));
  this->SetUserAction(new RMGManagementSteppingAction(event_action));
  this->SetUserAction(new RMGManagementTrackingAction(event_action));
}

// vim: tabstop=2 shiftwidth=2 expandtab

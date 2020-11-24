#include "MGManagementStackingAction.hh"

#include "MGManagementEventAction.hh"
#include "MGVOutputManager.hh"

MGManagementStackingAction::MGManagementStackingAction(MGManagementEventAction* eventaction) :
  fEventAction(eventaction) {}

G4ClassificationOfNewTrack MGManagementStackingAction::ClassifyNewTrack(const G4Track* aTrack) {
  if (fEventAction->GetOutputManager()) {
    return fEventAction->GetOutputManager()->StackingAction(aTrack);
  }
  else return fUrgent;
}

void MGManagementStackingAction::NewStage() {
  if (fEventAction->GetOutputManager()) {
    fEventAction->GetOutputManager()->NewStage();
  }
}

void MGManagementStackingAction::PrepareNewEvent() {
  if (fEventAction->GetOutputManager()) {
    fEventAction->GetOutputManager()->PrepareNewEvent();
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

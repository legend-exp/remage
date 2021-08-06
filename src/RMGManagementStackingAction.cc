#include "RMGManagementStackingAction.hh"

#include "RMGManagementEventAction.hh"
#include "RMGVOutputManager.hh"

RMGManagementStackingAction::RMGManagementStackingAction(RMGManagementEventAction* eventaction) :
  fEventAction(eventaction) {}

G4ClassificationOfNewTrack RMGManagementStackingAction::ClassifyNewTrack(const G4Track* /*aTrack*/) {
  // if (fEventAction->GetOutputManager()) {
  //   return fEventAction->GetOutputManager()->StackingAction(aTrack);
  // }
  // else return fUrgent;
  return fUrgent;
}

void RMGManagementStackingAction::NewStage() {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->NewStage();
  // }
}

void RMGManagementStackingAction::PrepareNewEvent() {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->PrepareNewEvent();
  // }
}

// vim: tabstop=2 shiftwidth=2 expandtab

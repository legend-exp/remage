#include "RMGStackingAction.hh"

#include "RMGEventAction.hh"
#include "RMGVOutputManager.hh"

RMGStackingAction::RMGStackingAction(RMGEventAction* eventaction) :
  fEventAction(eventaction) {}

G4ClassificationOfNewTrack RMGStackingAction::ClassifyNewTrack(const G4Track* /*aTrack*/) {
  // if (fEventAction->GetOutputManager()) {
  //   return fEventAction->GetOutputManager()->StackingAction(aTrack);
  // }
  // else return fUrgent;
  return fUrgent;
}

void RMGStackingAction::NewStage() {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->NewStage();
  // }
}

void RMGStackingAction::PrepareNewEvent() {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->PrepareNewEvent();
  // }
}

// vim: tabstop=2 shiftwidth=2 expandtab

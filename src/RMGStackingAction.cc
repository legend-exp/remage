#include "RMGStackingAction.hh"

#include "RMGEventAction.hh"

RMGStackingAction::RMGStackingAction(RMGEventAction* eventaction) : fEventAction(eventaction) {}

G4ClassificationOfNewTrack RMGStackingAction::ClassifyNewTrack(const G4Track* /*aTrack*/) {
  return fUrgent;
}

void RMGStackingAction::NewStage() {}

void RMGStackingAction::PrepareNewEvent() {}

// vim: tabstop=2 shiftwidth=2 expandtab

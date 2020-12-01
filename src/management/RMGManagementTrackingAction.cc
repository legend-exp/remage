#include "RMGManagementTrackingAction.hh"

#include "RMGManagementEventAction.hh"
#include "G4Track.hh"
#include "RMGVOutputManager.hh"

RMGManagementTrackingAction::RMGManagementTrackingAction(RMGManagementEventAction* eventaction) :
  fEventAction(eventaction) {}

void RMGManagementTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {
  if (fEventAction->GetOutputManager()) {
    fEventAction->GetOutputManager()->PreUserTrackingAction(aTrack);
  }
}

void RMGManagementTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
  if (fEventAction->GetOutputManager()) {
    fEventAction->GetOutputManager()->PostUserTrackingAction(aTrack);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

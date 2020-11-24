#include "MGManagementTrackingAction.hh"

#include "MGManagementEventAction.hh"
#include "G4Track.hh"
#include "MGVOutputManager.hh"

MGManagementTrackingAction::MGManagementTrackingAction(MGManagementEventAction* eventaction) :
  fEventAction(eventaction) {}

void MGManagementTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {
  if (fEventAction->GetOutputManager()) {
    fEventAction->GetOutputManager()->PreUserTrackingAction(aTrack);
  }
}

void MGManagementTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
  if (fEventAction->GetOutputManager()) {
    fEventAction->GetOutputManager()->PostUserTrackingAction(aTrack);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

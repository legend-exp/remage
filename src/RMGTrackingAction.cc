#include "RMGTrackingAction.hh"

#include "RMGEventAction.hh"
#include "G4Track.hh"
#include "RMGVOutputManager.hh"

RMGTrackingAction::RMGTrackingAction(RMGEventAction* eventaction) :
  fEventAction(eventaction) {}

void RMGTrackingAction::PreUserTrackingAction(const G4Track* /*aTrack*/) {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->PreUserTrackingAction(aTrack);
  // }
}

void RMGTrackingAction::PostUserTrackingAction(const G4Track* /*aTrack*/) {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->PostUserTrackingAction(aTrack);
  // }
}

// vim: tabstop=2 shiftwidth=2 expandtab

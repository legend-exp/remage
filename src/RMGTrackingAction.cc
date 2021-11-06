#include "RMGTrackingAction.hh"

#include "RMGEventAction.hh"
#include "G4Track.hh"

RMGTrackingAction::RMGTrackingAction(RMGEventAction* eventaction) :
  fEventAction(eventaction) {}

void RMGTrackingAction::PreUserTrackingAction(const G4Track* /*aTrack*/) {
}

void RMGTrackingAction::PostUserTrackingAction(const G4Track* /*aTrack*/) {
}

// vim: tabstop=2 shiftwidth=2 expandtab

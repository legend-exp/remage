#include "RMGSteppingAction.hh"

#include "G4Step.hh"

#include "RMGEventAction.hh"

RMGSteppingAction::RMGSteppingAction(RMGEventAction* eventaction):
  fEventAction(eventaction) {}

void RMGSteppingAction::UserSteppingAction(const G4Step* /*step*/) {
}

// vim: tabstop=2 shiftwidth=2 expandtab

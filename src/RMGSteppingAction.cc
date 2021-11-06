#include "RMGSteppingAction.hh"

#include "G4Step.hh"

#include "RMGEventAction.hh"
#include "RMGVOutputManager.hh"

RMGSteppingAction::RMGSteppingAction(RMGEventAction* eventaction):
  fEventAction(eventaction) {}

void RMGSteppingAction::UserSteppingAction(const G4Step* /*step*/) {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->SteppingAction(step, G4UserSteppingAction::fpSteppingManager);
  // }
}

// vim: tabstop=2 shiftwidth=2 expandtab

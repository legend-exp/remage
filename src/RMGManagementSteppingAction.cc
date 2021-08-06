#include "RMGManagementSteppingAction.hh"

#include "G4Step.hh"

#include "RMGManagementEventAction.hh"
#include "RMGVOutputManager.hh"

RMGManagementSteppingAction::RMGManagementSteppingAction(RMGManagementEventAction* eventaction):
  fEventAction(eventaction) {}

void RMGManagementSteppingAction::UserSteppingAction(const G4Step* /*step*/) {
  // if (fEventAction->GetOutputManager()) {
  //   fEventAction->GetOutputManager()->SteppingAction(step, G4UserSteppingAction::fpSteppingManager);
  // }
}

// vim: tabstop=2 shiftwidth=2 expandtab

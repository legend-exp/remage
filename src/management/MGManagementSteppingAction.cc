#include "MGManagementSteppingAction.hh"

#include "G4Step.hh"

#include "MGManagementEventAction.hh"
#include "MGVOutputManager.hh"

MGManagementSteppingAction::MGManagementSteppingAction(MGManagementEventAction * eventaction):
  fEventAction(eventaction) {}

void MGManagementSteppingAction::UserSteppingAction(const G4Step *step) {
  if (fEventAction->GetOutputManager()) {
    fEventAction->GetOutputManager()->SteppingAction(step, fpSteppingManager);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

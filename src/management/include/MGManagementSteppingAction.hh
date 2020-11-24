#ifndef _MGMANAGEMENTSTEPPINGACTION_HH_
#define _MGMANAGEMENTSTEPPINGACTION_HH_

#include "G4UserSteppingAction.hh"

class G4Step;
class MGManagementEventAction;
class MGManagementSteppingAction : public G4UserSteppingAction {

  public:

    MGManagementSteppingAction(MGManagementEventAction* eventaction);
    ~MGManagementSteppingAction() = default;

    MGManagementSteppingAction           (MGManagementSteppingAction const&) = delete;
    MGManagementSteppingAction& operator=(MGManagementSteppingAction const&) = delete;
    MGManagementSteppingAction           (MGManagementSteppingAction&&)      = delete;
    MGManagementSteppingAction& operator=(MGManagementSteppingAction&&)      = delete;

    void UserSteppingAction(const G4Step *step) override;

  private:

    MGManagementEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

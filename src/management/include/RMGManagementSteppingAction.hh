#ifndef _RMGMANAGEMENTSTEPPINGACTION_HH_
#define _RMGMANAGEMENTSTEPPINGACTION_HH_

#include "G4UserSteppingAction.hh"

class G4Step;
class RMGManagementEventAction;
class RMGManagementSteppingAction : public G4UserSteppingAction {

  public:

    RMGManagementSteppingAction(RMGManagementEventAction* eventaction);
    ~RMGManagementSteppingAction() = default;

    RMGManagementSteppingAction           (RMGManagementSteppingAction const&) = delete;
    RMGManagementSteppingAction& operator=(RMGManagementSteppingAction const&) = delete;
    RMGManagementSteppingAction           (RMGManagementSteppingAction&&)      = delete;
    RMGManagementSteppingAction& operator=(RMGManagementSteppingAction&&)      = delete;

    void UserSteppingAction(const G4Step *step) override;

  private:

    RMGManagementEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

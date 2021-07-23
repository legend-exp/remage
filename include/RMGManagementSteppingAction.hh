#ifndef _RMG_MANAGEMENT_STEPPING_ACTION_HH_
#define _RMG_MANAGEMENT_STEPPING_ACTION_HH_

#include "G4UserSteppingAction.hh"

class G4Step;
class RMGManagementEventAction;
class RMGManagementSteppingAction : public G4UserSteppingAction {

  public:

    RMGManagementSteppingAction(RMGManagementEventAction*);
    ~RMGManagementSteppingAction() = default;

    RMGManagementSteppingAction           (RMGManagementSteppingAction const&) = delete;
    RMGManagementSteppingAction& operator=(RMGManagementSteppingAction const&) = delete;
    RMGManagementSteppingAction           (RMGManagementSteppingAction&&)      = delete;
    RMGManagementSteppingAction& operator=(RMGManagementSteppingAction&&)      = delete;

    void UserSteppingAction(const G4Step*) override;

  private:

    RMGManagementEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

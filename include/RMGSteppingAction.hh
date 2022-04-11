#ifndef _RMG_MANAGEMENT_STEPPING_ACTION_HH_
#define _RMG_MANAGEMENT_STEPPING_ACTION_HH_

#include "G4UserSteppingAction.hh"

class G4Step;
class RMGEventAction;
class RMGSteppingAction : public G4UserSteppingAction {

  public:

    RMGSteppingAction(RMGEventAction*);
    ~RMGSteppingAction() = default;

    RMGSteppingAction(RMGSteppingAction const&) = delete;
    RMGSteppingAction& operator=(RMGSteppingAction const&) = delete;
    RMGSteppingAction(RMGSteppingAction&&) = delete;
    RMGSteppingAction& operator=(RMGSteppingAction&&) = delete;

    void UserSteppingAction(const G4Step*) override;

  private:

    RMGEventAction* fEventAction = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

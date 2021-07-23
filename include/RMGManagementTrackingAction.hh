#ifndef _RMG_MANAGEMENT_TRACKING_ACTION_HH_
#define _RMG_MANAGEMENT_TRACKING_ACTION_HH_

#include "G4UserTrackingAction.hh"

class RMGManagementEventAction;
class RMGManagementTrackingAction : public G4UserTrackingAction {

  public:

    RMGManagementTrackingAction(RMGManagementEventAction*);
    ~RMGManagementTrackingAction() = default;

    RMGManagementTrackingAction           (RMGManagementTrackingAction const&) = delete;
    RMGManagementTrackingAction& operator=(RMGManagementTrackingAction const&) = delete;
    RMGManagementTrackingAction           (RMGManagementTrackingAction&&)      = delete;
    RMGManagementTrackingAction& operator=(RMGManagementTrackingAction&&)      = delete;

    virtual void PreUserTrackingAction(const G4Track*) override;
    virtual void PostUserTrackingAction(const G4Track*) override;
    inline G4TrackingManager* GetTrackingManager() { return G4UserTrackingAction::fpTrackingManager; };

  private:

    RMGManagementEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

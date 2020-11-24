#ifndef _MGMANAGEMENTTRACKINGACTION_HH_
#define _MGMANAGEMENTTRACKINGACTION_HH_

#include "G4UserTrackingAction.hh"

class MGManagementEventAction;
class MGManagementTrackingAction : public G4UserTrackingAction {

  public:

    MGManagementTrackingAction(MGManagementEventAction*);
    virtual ~MGManagementTrackingAction() = default;

    MGManagementTrackingAction           (MGManagementTrackingAction const&) = delete;
    MGManagementTrackingAction& operator=(MGManagementTrackingAction const&) = delete;
    MGManagementTrackingAction           (MGManagementTrackingAction&&)      = delete;
    MGManagementTrackingAction& operator=(MGManagementTrackingAction&&)      = delete;

    virtual void PreUserTrackingAction(const G4Track*) override;
    virtual void PostUserTrackingAction(const G4Track*) override;
    inline G4TrackingManager* GetTrackingManager() { return G4UserTrackingAction::fpTrackingManager; };

  private:

    MGManagementEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

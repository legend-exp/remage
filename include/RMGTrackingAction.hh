#ifndef _RMG_MANAGEMENT_TRACKING_ACTION_HH_
#define _RMG_MANAGEMENT_TRACKING_ACTION_HH_

#include "G4UserTrackingAction.hh"

class RMGEventAction;
class RMGTrackingAction : public G4UserTrackingAction {

  public:

    RMGTrackingAction(RMGEventAction*);
    ~RMGTrackingAction() = default;

    RMGTrackingAction(RMGTrackingAction const&) = delete;
    RMGTrackingAction& operator=(RMGTrackingAction const&) = delete;
    RMGTrackingAction(RMGTrackingAction&&) = delete;
    RMGTrackingAction& operator=(RMGTrackingAction&&) = delete;

    virtual void PreUserTrackingAction(const G4Track*) override;
    virtual void PostUserTrackingAction(const G4Track*) override;
    inline G4TrackingManager* GetTrackingManager() {
      return G4UserTrackingAction::fpTrackingManager;
    };

  private:

    RMGEventAction* fEventAction = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

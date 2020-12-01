#ifndef _RMGMANAGEMENTSTACKINGACTION_HH_
#define _RMGMANAGEMENTSTACKINGACTION_HH_

#include "G4UserStackingAction.hh"

class G4Track;
class RMGManagementEventAction;
class RMGManagementStackingAction : public G4UserStackingAction {

  public:

    RMGManagementStackingAction(RMGManagementEventAction *eventaction);
    ~RMGManagementStackingAction() = default;

    RMGManagementStackingAction           (RMGManagementStackingAction const&) = delete;
    RMGManagementStackingAction& operator=(RMGManagementStackingAction const&) = delete;
    RMGManagementStackingAction           (RMGManagementStackingAction&&)      = delete;
    RMGManagementStackingAction& operator=(RMGManagementStackingAction&&)      = delete;

    G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
    void NewStage() override;
    void PrepareNewEvent() override;

  private:

    RMGManagementEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

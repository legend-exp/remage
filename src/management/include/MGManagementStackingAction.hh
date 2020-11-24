#ifndef _MGMANAGEMENTSTACKINGACTION_HH_
#define _MGMANAGEMENTSTACKINGACTION_HH_

#include "G4UserStackingAction.hh"

class G4Track;
class MGManagementEventAction;
class MGManagementStackingAction : public G4UserStackingAction {

  public:

    MGManagementStackingAction(MGManagementEventAction *eventaction);
    ~MGManagementStackingAction() = default;

    MGManagementStackingAction           (MGManagementStackingAction const&) = delete;
    MGManagementStackingAction& operator=(MGManagementStackingAction const&) = delete;
    MGManagementStackingAction           (MGManagementStackingAction&&)      = delete;
    MGManagementStackingAction& operator=(MGManagementStackingAction&&)      = delete;

    G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
    void NewStage() override;
    void PrepareNewEvent() override;

  private:

    MGManagementEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

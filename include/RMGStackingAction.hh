#ifndef _RMG_MANAGEMENT_STACKING_ACTION_HH_
#define _RMG_MANAGEMENT_STACKING_ACTION_HH_

#include "G4UserStackingAction.hh"

class G4Track;
class RMGEventAction;
class RMGStackingAction : public G4UserStackingAction {

  public:

    RMGStackingAction(RMGEventAction*);
    ~RMGStackingAction() = default;

    RMGStackingAction           (RMGStackingAction const&) = delete;
    RMGStackingAction& operator=(RMGStackingAction const&) = delete;
    RMGStackingAction           (RMGStackingAction&&)      = delete;
    RMGStackingAction& operator=(RMGStackingAction&&)      = delete;

    G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
    void NewStage() override;
    void PrepareNewEvent() override;

  private:

    RMGEventAction* fEventAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMG_MANAGEMENT_USER_ACTION_INITIALIZATION_HH_
#define _RMG_MANAGEMENT_USER_ACTION_INITIALIZATION_HH_

#include <memory>

#include "globals.hh"

#include "G4VUserActionInitialization.hh"

// class RMGGeneratorPrimary;
// class RMGManagementEventAction;
// class RMGManagementRunAction;
// class RMGManagementSteppingAction;
// class RMGManagementTrackingAction;
// class RMGManagementStackingAction;
class RMGManagementUserActionInitialization : public G4VUserActionInitialization {

  public:

    RMGManagementUserActionInitialization() = default;
    ~RMGManagementUserActionInitialization() override = default;

    RMGManagementUserActionInitialization           (RMGManagementUserActionInitialization const&) = delete;
    RMGManagementUserActionInitialization& operator=(RMGManagementUserActionInitialization const&) = delete;
    RMGManagementUserActionInitialization           (RMGManagementUserActionInitialization&&)      = delete;
    RMGManagementUserActionInitialization& operator=(RMGManagementUserActionInitialization&&)      = delete;

    void Build() const override;
    void BuildForMaster() const override;

  private:

    // RMGGeneratorPrimary*             fGeneratorPrimary;
    // RMGManagementEventAction*        fManagementEventAction;
    // RMGManagementRunAction*          fManagementRunAction;
    // RMGManagementSteppingAction*     fManagementSteppingAction;
    // RMGManagementTrackingAction*     fManagementTrackingAction;
    // RMGManagementStackingAction*     fManagementStackingAction;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

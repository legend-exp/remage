#ifndef _RMG_MANAGEMENT_USER_ACTION_HH_
#define _RMG_MANAGEMENT_USER_ACTION_HH_

#include "globals.hh"

#include "G4VUserActionInitialization.hh"

class RMGManagementUserAction : public G4VUserActionInitialization {

  public:

    inline RMGManagementUserAction() = default;
    ~RMGManagementUserAction() override = default;

    RMGManagementUserAction           (RMGManagementUserAction const&) = delete;
    RMGManagementUserAction& operator=(RMGManagementUserAction const&) = delete;
    RMGManagementUserAction           (RMGManagementUserAction&&)      = delete;
    RMGManagementUserAction& operator=(RMGManagementUserAction&&)      = delete;

    void Build() const override;
    void BuildForMaster() const override;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

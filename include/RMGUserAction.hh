#ifndef _RMG_MANAGEMENT_USER_ACTION_HH_
#define _RMG_MANAGEMENT_USER_ACTION_HH_

#include "G4VUserActionInitialization.hh"

class RMGUserAction : public G4VUserActionInitialization {

  public:

    inline RMGUserAction() = default;
    ~RMGUserAction() override = default;

    RMGUserAction(RMGUserAction const&) = delete;
    RMGUserAction& operator=(RMGUserAction const&) = delete;
    RMGUserAction(RMGUserAction&&) = delete;
    RMGUserAction& operator=(RMGUserAction&&) = delete;

    void Build() const override;
    void BuildForMaster() const override;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

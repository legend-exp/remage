#ifndef _RMGMANAGEMENTRUNACTION_HH_
#define _RMGMANAGEMENTRUNACTION_HH_

#include "globals.hh"
#include <ctime>

#include "G4UserRunAction.hh"

class G4Run;
class RMGManager;
class RMGManagementRunAction : public G4UserRunAction {

  public:

    RMGManagementRunAction();
    ~RMGManagementRunAction() = default;

    RMGManagementRunAction           (RMGManagementRunAction const&) = delete;
    RMGManagementRunAction& operator=(RMGManagementRunAction const&) = delete;
    RMGManagementRunAction           (RMGManagementRunAction&&)      = delete;
    RMGManagementRunAction& operator=(RMGManagementRunAction&&)      = delete;

    void BeginOfRunAction(const G4Run* run) override;
    void EndOfRunAction(const G4Run* run) override;

    void SetControlledRandomization() { fControlledRandomization = true; }
    G4bool GetControlledRandomization() { return fControlledRandomization; }
    time_t GetStartTime() { return fStartTime; }

  private:

    G4bool fControlledRandomization;
    time_t fStartTime;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMGMANAGEMENTRUNACTION_HH_
#define _RMGMANAGEMENTRUNACTION_HH_

#include <memory>
#include <chrono>

#include "globals.hh"
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

    inline void SetControlledRandomization() { fControlledRandomization = true; }
    inline G4bool GetControlledRandomization() { return fControlledRandomization; }
    inline const std::chrono::time_point<std::chrono::system_clock>& GetStartTime() { return fStartTime; }

  private:

    G4bool fControlledRandomization;
    std::chrono::time_point<std::chrono::system_clock> fStartTime;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

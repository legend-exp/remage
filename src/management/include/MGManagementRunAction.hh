#ifndef _MGMANAGEMENTRUNACTION_HH_
#define _MGMANAGEMENTRUNACTION_HH_

#include "globals.hh"
#include <ctime>

#include "G4UserRunAction.hh"

class G4Run;
class MGManager;
class MGManagementRunAction : public G4UserRunAction {

  public:

    MGManagementRunAction();
    ~MGManagementRunAction() = default;

    MGManagementRunAction           (MGManagementRunAction const&) = delete;
    MGManagementRunAction& operator=(MGManagementRunAction const&) = delete;
    MGManagementRunAction           (MGManagementRunAction&&)      = delete;
    MGManagementRunAction& operator=(MGManagementRunAction&&)      = delete;

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

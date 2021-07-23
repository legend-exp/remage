#ifndef _RMG_MANAGEMENT_RUN_ACTION_HH_
#define _RMG_MANAGEMENT_RUN_ACTION_HH_

#include <chrono>

#include "globals.hh"
#include "G4UserRunAction.hh"

class G4Run;
class RMGRun;
class RMGGeneratorPrimary;
class RMGManagementRunAction : public G4UserRunAction {

  public:

    RMGManagementRunAction() = default;
    RMGManagementRunAction(RMGGeneratorPrimary*);
    ~RMGManagementRunAction() = default;

    RMGManagementRunAction           (RMGManagementRunAction const&) = delete;
    RMGManagementRunAction& operator=(RMGManagementRunAction const&) = delete;
    RMGManagementRunAction           (RMGManagementRunAction&&)      = delete;
    RMGManagementRunAction& operator=(RMGManagementRunAction&&)      = delete;

    G4Run* GenerateRun() override;
    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

  private:

    RMGRun* fRMGRun;
    RMGGeneratorPrimary* fRMGGeneratorPrimary;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

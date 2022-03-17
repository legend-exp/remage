#ifndef _RMG_MANAGEMENT_RUN_ACTION_HH_
#define _RMG_MANAGEMENT_RUN_ACTION_HH_

#include <memory>
#include <chrono>
#include <vector>
#include <map>

#include "G4UserRunAction.hh"
#include "G4AnalysisManager.hh"

#include "RMGHardware.hh"
#include "RMGVOutputScheme.hh"

class G4Run;
class RMGRun;
class RMGMasterGenerator;
class RMGRunAction : public G4UserRunAction {

  public:

    RMGRunAction(bool persistency=false);
    RMGRunAction(RMGMasterGenerator*, bool persistency=false);
    ~RMGRunAction() = default;

    RMGRunAction           (RMGRunAction const&) = delete;
    RMGRunAction& operator=(RMGRunAction const&) = delete;
    RMGRunAction           (RMGRunAction&&)      = delete;
    RMGRunAction& operator=(RMGRunAction&&)      = delete;

    void SetupAnalysisManager();
    G4Run* GenerateRun() override;
    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    inline auto& GetOutputDataFields(RMGHardware::DetectorType d_type) {
      return fOutputDataFields.at(d_type);
    }
    inline void ClearOutputDataFields() {
      for (auto& el : fOutputDataFields) el.second->clear();
    }

  private:

    RMGRun* fRMGRun = nullptr;
    bool fIsPersistencyEnabled = false;
    RMGMasterGenerator* fRMGMasterGenerator = nullptr;

    std::map<RMGHardware::DetectorType, std::unique_ptr<RMGVOutputScheme>> fOutputDataFields;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _RMG_RUN_ACTION_HH_
#define _RMG_RUN_ACTION_HH_

#include <chrono>
#include <map>
#include <memory>
#include <vector>

#include "G4AnalysisManager.hh"
#include "G4UserRunAction.hh"

#include "RMGHardware.hh"
#include "RMGVOutputScheme.hh"

class G4Run;
class RMGRun;
class RMGMasterGenerator;
class RMGRunAction : public G4UserRunAction {

  public:

    RMGRunAction(bool persistency = false);
    RMGRunAction(RMGMasterGenerator*, bool persistency = false);
    ~RMGRunAction() = default;

    RMGRunAction(RMGRunAction const&) = delete;
    RMGRunAction& operator=(RMGRunAction const&) = delete;
    RMGRunAction(RMGRunAction&&) = delete;
    RMGRunAction& operator=(RMGRunAction&&) = delete;

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
    bool fIsAnaManInitialized = false;
    RMGMasterGenerator* fRMGMasterGenerator = nullptr;

    std::map<RMGHardware::DetectorType, std::unique_ptr<RMGVOutputScheme>> fOutputDataFields;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

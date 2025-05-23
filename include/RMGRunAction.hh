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

#include <filesystem>
#include <memory>
#include <vector>
namespace fs = std::filesystem;

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

    [[nodiscard]] int GetCurrentRunPrintModulo() const { return fCurrentPrintModulo; }

    [[nodiscard]] const auto& GetAllOutputDataFields() { return fOutputDataFields; }
    void ClearOutputDataFields() {
      for (auto& el : fOutputDataFields) el->ClearBeforeEvent();
    }

  private:

    [[nodiscard]] std::pair<fs::path, fs::path> BuildOutputFile() const;
    void PostprocessOutputFile() const;

    RMGRun* fRMGRun = nullptr;
    bool fIsPersistencyEnabled = false;
    bool fIsAnaManInitialized = false;
    RMGMasterGenerator* fRMGMasterGenerator = nullptr;
    // a pair containing the actual Geant4 (temporary) file, and the original file name;
    // possibly being equal.
    std::pair<fs::path, fs::path> fCurrentOutputFile;

    int fCurrentPrintModulo = -1;

    std::vector<std::shared_ptr<RMGVOutputScheme>> fOutputDataFields;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

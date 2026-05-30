// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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
/**
 * @brief Per-thread run action managing output files, ntuples and output schemes.
 *
 * Holds the list of @ref RMGVOutputScheme instances active on this thread, opens and
 * post-processes the per-worker output file (writing to a temporary path first and moving
 * it into place at end of run), and initializes the Geant4 analysis manager.
 */
class RMGRunAction : public G4UserRunAction {

  public:

    /** @brief Construct a run action with no primary generator hook-up. */
    RMGRunAction(bool persistency = false);
    /** @brief Construct a run action aware of the primary generator. */
    RMGRunAction(RMGMasterGenerator*, bool persistency = false);
    ~RMGRunAction() = default;

    RMGRunAction(RMGRunAction const&) = delete;
    RMGRunAction& operator=(RMGRunAction const&) = delete;
    RMGRunAction(RMGRunAction&&) = delete;
    RMGRunAction& operator=(RMGRunAction&&) = delete;

    /**
     * @brief Configure the Geant4 analysis manager and register all output ntuples.
     * @details subsequent calls are no-ops once the analysis manager is initialized
     * for the current thread.
     */
    void SetupAnalysisManager();
    /** @brief Allocate and return an @ref RMGRun instance for the current run. */
    G4Run* GenerateRun() override;
    /** @brief Open the output file, notify schemes, and record the run start time. */
    void BeginOfRunAction(const G4Run*) override;
    /** @brief Close the output file and move the temporary worker file to its final path. */
    void EndOfRunAction(const G4Run*) override;

    /** @brief Print-modulo value chosen for the run currently being processed. */
    [[nodiscard]] int GetCurrentRunPrintModulo() const { return fCurrentPrintModulo; }

    /** @brief Output schemes registered on this thread. */
    [[nodiscard]] const auto& GetAllOutputDataFields() { return fOutputDataFields; }
    /** @brief Invoke @ref RMGVOutputScheme::ClearBeforeEvent on every registered scheme. */
    void ClearOutputDataFields() {
      for (auto& el : fOutputDataFields) el->ClearBeforeEvent();
    }

  private:

    /** @brief struct containing the actual Geant4 (temporary) file, and the original file name;
     *  possibly being equal. */
    struct OutputFilePaths {
        fs::path tmp;
        fs::path original;
    };

    [[nodiscard]] OutputFilePaths BuildOutputFile() const;
    [[nodiscard]] fs::path GetWorkerTmpPath(fs::path path, std::string extension) const;
    void PostprocessOutputFile(int number_of_primaries) const;

    RMGRun* fRMGRun = nullptr;
    bool fIsPersistencyEnabled = false;
    bool fIsAnaManInitialized = false;
    RMGMasterGenerator* fRMGMasterGenerator = nullptr;
    OutputFilePaths fCurrentOutputFile;

    int fCurrentPrintModulo = -1;

    std::vector<std::shared_ptr<RMGVOutputScheme>> fOutputDataFields;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

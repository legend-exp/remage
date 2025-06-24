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

#ifndef _RMG_OUTPUT_MANAGER_HH_
#define _RMG_OUTPUT_MANAGER_HH_

#include <atomic>
#include <memory>
#include <set>
#include <vector>

#include "G4AnalysisManager.hh"
#include "G4Threading.hh"
#include "globals.hh"

#include "RMGLog.hh"

class G4GenericMessenger;
class RMGOutputManager {

  public:

    RMGOutputManager();
    ~RMGOutputManager() = default;

    RMGOutputManager(RMGOutputManager const&) = delete;
    RMGOutputManager& operator=(RMGOutputManager const&) = delete;
    RMGOutputManager(RMGOutputManager&&) = delete;
    RMGOutputManager& operator=(RMGOutputManager&&) = delete;

    // getters
    static RMGOutputManager* Instance() {
      if (!fRMGOutputManager) fRMGOutputManager = new RMGOutputManager();
      return fRMGOutputManager;
    }

    [[nodiscard]] bool IsPersistencyEnabled() const { return fIsPersistencyEnabled; }
    const std::string& GetOutputFileName() { return fOutputFile; }
    [[nodiscard]] bool HasOutputFileNameNone() const { return fOutputFile == OUTPUT_FILE_NONE; }
    [[nodiscard]] bool HasOutputFileName() const {
      return !fOutputFile.empty() && fOutputFile != OUTPUT_FILE_NONE;
    }
    [[nodiscard]] bool GetOutputOverwriteFiles() const { return fOutputOverwriteFiles; }
    const std::string& GetOutputNtupleDirectory() { return fOutputNtupleDirectory; }
    [[nodiscard]] bool GetOutputNtuplePerDetector() const { return fOutputNtuplePerDetector; }
    [[nodiscard]] bool GetOutputNtupleUseVolumeName() const { return fOutputNtupleUseVolumeName; }

    std::set<int> GetNtupleNames() {
      std::set<int> nt_names;
      for (auto const& [k, v] : fNtupleIDs) nt_names.insert(k);
      return nt_names;
    }
    std::set<std::string> GetAuxNtupleNames() {
      std::set<std::string> nt_names;
      for (auto const& [k, v] : fNtupleAuxIDs) nt_names.insert(k);
      return nt_names;
    }

    // setters
    void EnablePersistency(bool flag = true) { fIsPersistencyEnabled = flag; }

    void SetOutputFileName(std::string filename) { fOutputFile = filename; }
    void SetOutputOverwriteFiles(bool overwrite) { fOutputOverwriteFiles = overwrite; }
    void SetOutputNtupleDirectory(std::string dir) { fOutputNtupleDirectory = dir; }

    int RegisterNtuple(int det_uid, int ntuple_id);
    int CreateAndRegisterNtuple(
        int det_uid,
        std::string table_name,
        std::string oscheme,
        G4AnalysisManager* ana_man
    );
    int CreateAndRegisterAuxNtuple(
        std::string table_name,
        std::string oscheme,
        G4AnalysisManager* ana_man
    );
    int GetNtupleID(int det_uid) { return fNtupleIDs[det_uid]; }
    int GetAuxNtupleID(std::string det_uid) { return fNtupleAuxIDs[det_uid]; }

    // NOLINTNEXTLINE(readability-make-member-function-const)
    void ActivateOptionalOutputScheme(std::string name);

  private:

    static inline const std::string OUTPUT_FILE_NONE = "none";
    std::string fOutputFile;
    bool fIsPersistencyEnabled = true;
    bool fOutputOverwriteFiles = false;
    bool fOutputNtuplePerDetector = true;
    bool fOutputNtupleUseVolumeName = false;
    std::string fOutputNtupleDirectory = "stp";
    // track internal id of detector NTuples
    static G4ThreadLocal std::map<int, int> fNtupleIDs;
    static G4ThreadLocal std::map<std::string, int> fNtupleAuxIDs;

    static RMGOutputManager* fRMGOutputManager;

    // messenger stuff
    std::unique_ptr<G4GenericMessenger> fOutputMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

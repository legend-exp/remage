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

#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

#include <atomic>
#include <memory>
#include <vector>

#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4Threading.hh"
#include "G4VisManager.hh"
#include "globals.hh"

#include "RMGExceptionHandler.hh"
#include "RMGLog.hh"
#include "RMGUserInit.hh"

class G4VUserPhysicsList;
class RMGHardware;
class RMGUserAction;
class G4GenericMessenger;
class G4UIExecutive;
class RMGManager {

  public:

    RMGManager() = delete;
    RMGManager(std::string app_name, int argc, char** argv);
    ~RMGManager() = default;

    RMGManager(RMGManager const&) = delete;
    RMGManager& operator=(RMGManager const&) = delete;
    RMGManager(RMGManager&&) = delete;
    RMGManager& operator=(RMGManager&&) = delete;

    // getters
    static RMGManager* Instance() { return fRMGManager; }
    G4RunManager* GetG4RunManager();
    G4VisManager* GetG4VisManager();
    RMGHardware* GetDetectorConstruction();
    G4VUserPhysicsList* GetProcessesList();
    [[nodiscard]] auto GetUserInit() const { return fUserInit; }
    [[nodiscard]] int GetPrintModulo() const { return fPrintModulo; }

    [[nodiscard]] bool IsExecSequential() {
      return fG4RunManager->GetRunManagerType() == G4RunManager::RMType::sequentialRM;
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

    // setters
    void SetUserInit(G4RunManager* g4_manager) {
      fG4RunManager = std::unique_ptr<G4RunManager>(g4_manager);
    }
    void SetUserInit(G4VisManager* vis) { fG4VisManager = std::unique_ptr<G4VisManager>(vis); }
    void SetUserInit(RMGHardware* det) { fDetectorConstruction = det; }
    void SetUserInit(G4VUserPhysicsList* proc) { fPhysicsList = proc; }

    void SetInteractive(bool flag = true) { fInteractive = flag; }
    void SetNumberOfThreads(int nthreads) { fNThreads = nthreads; }
    void SetPrintModulo(int n_ev) { fPrintModulo = n_ev > 0 ? n_ev : -1; }

    void EnablePersistency(bool flag = true) { fIsPersistencyEnabled = flag; }
    void IncludeMacroFile(std::string filename) { fMacroFilesOrContents.emplace_back(filename); }
    void RegisterG4Alias(std::string alias, std::string value) { fG4Aliases.emplace(alias, value); }
    void Initialize();
    void Run();

    void SetRandEngine(std::string name);
    void SetRandEngineSeed(int seed);
    void SetRandEngineInternalSeed(int index);
    void SetRandSystemEntropySeed();
    bool ApplyRandEngineForCurrentThread();
    [[nodiscard]] bool GetRandIsControlled() const { return fIsRandControlled; }
    [[nodiscard]] bool GetRandEngineSelected() const { return !fRandEngineName.empty(); }

    void SetLogLevel(std::string level);

    void SetOutputFileName(std::string filename) { fOutputFile = filename; }
    void SetOutputOverwriteFiles(bool overwrite) { fOutputOverwriteFiles = overwrite; }
    void SetOutputNtupleDirectory(std::string dir) { fOutputNtupleDirectory = dir; }
    int RegisterNtuple(int det_uid, int ntuple_id) {
      auto res = fNtupleIDs.emplace(det_uid, ntuple_id);
      if (!res.second)
        RMGLog::OutFormatDev(
            RMGLog::fatal,
            "Ntuple for detector with UID {} is already registered",
            det_uid
        );
      return this->GetNtupleID(det_uid);
    }
    int RegisterNtuple(std::string det_uid, int ntuple_id) {
      auto res = fNtupleIDsS.emplace(det_uid, ntuple_id);
      if (!res.second)
        RMGLog::OutFormatDev(
            RMGLog::fatal,
            "Ntuple for detector with UID {} is already registered",
            det_uid
        );
      return this->GetNtupleID(det_uid);
    }
    int GetNtupleID(std::string det_uid) { return fNtupleIDsS[det_uid]; }
    int GetNtupleID(int det_uid) { return fNtupleIDs[det_uid]; }

    [[nodiscard]] bool HadWarning() const {
      return fExceptionHandler->HadWarning() || RMGLog::HadWarning();
    }
    [[nodiscard]] bool HadError() const {
      return fExceptionHandler->HadError() || RMGLog::HadError();
    }

    // NOLINTNEXTLINE(readability-make-member-function-const)
    void ActivateOptionalOutputScheme(std::string name) {
      GetUserInit()->ActivateOptionalOutputScheme(name);
    }

    static void AbortRunGracefully() {
      if (!fAbortRun.is_lock_free()) return;
      fAbortRun = true;
    }
    static bool ShouldAbortRun() { return fAbortRun; }

  private:

    void SetUpDefaultG4RunManager(G4RunManagerType type = G4RunManagerType::Default);
    void SetUpDefaultG4VisManager();
    void SetUpDefaultDetectorConstruction();
    void SetUpDefaultProcessesList();
    void SetUpDefaultUserAction();
    void CheckRandEngineMTState();

    std::unique_ptr<G4UIExecutive> StartInteractiveSession();

    std::string fApplicationName;
    int fArgc;
    char** fArgv;
    std::map<std::string, std::string> fG4Aliases;
    std::vector<std::string> fMacroFilesOrContents;
    bool fInteractive = false;
    bool fIsPersistencyEnabled = true;
    int fPrintModulo = -1;
    int fNThreads = 1;

    bool fIsRandControlled = false;
    bool fIsRandControlledAtEngineChange = false;
    std::string fRandEngineName;

    static inline const std::string OUTPUT_FILE_NONE = "none";
    std::string fOutputFile;
    bool fOutputOverwriteFiles = false;
    bool fOutputNtuplePerDetector = true;
    bool fOutputNtupleUseVolumeName = false;
    std::string fOutputNtupleDirectory = "stp";
    // track internal id of detector NTuples
    static G4ThreadLocal std::map<int, int> fNtupleIDs;
    static G4ThreadLocal std::map<std::string, int> fNtupleIDsS;


    static RMGManager* fRMGManager;
    static std::atomic<bool> fAbortRun;

    std::unique_ptr<G4RunManager> fG4RunManager = nullptr;
    std::unique_ptr<G4VisManager> fG4VisManager = nullptr;

    RMGExceptionHandler* fExceptionHandler = nullptr;
    G4VUserPhysicsList* fPhysicsList = nullptr;
    RMGHardware* fDetectorConstruction = nullptr;

    RMGUserAction* fUserAction = nullptr;
    // store partial custom actions to be used later in RMGUserAction
    std::shared_ptr<RMGUserInit> fUserInit;

    // messenger stuff
    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<G4GenericMessenger> fLogMessenger;
    std::unique_ptr<G4GenericMessenger> fRandMessenger;
    std::unique_ptr<G4GenericMessenger> fOutputMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

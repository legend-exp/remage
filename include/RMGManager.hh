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

#include <memory>
#include <vector>

#include "globals.hh"

#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4Threading.hh"
#include "G4VisManager.hh"

#include "RMGLog.hh"

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
    static inline RMGManager* Instance() { return fRMGManager; }
    G4RunManager* GetG4RunManager();
    G4VisManager* GetG4VisManager();
    RMGHardware* GetDetectorConstruction();
    G4VUserPhysicsList* GetProcessesList();
    [[nodiscard]] inline int GetPrintModulo() const { return fPrintModulo; }

    inline bool IsExecSequential() {
      return fG4RunManager->GetRunManagerType() == G4RunManager::RMType::sequentialRM;
    }
    [[nodiscard]] inline bool IsPersistencyEnabled() const { return fIsPersistencyEnabled; }
    inline const std::string& GetOutputFileName() { return fOutputFile; }

    // setters
    inline void SetUserInit(G4RunManager* g4_manager) {
      fG4RunManager = std::unique_ptr<G4RunManager>(g4_manager);
    }
    inline void SetUserInit(G4VisManager* vis) {
      fG4VisManager = std::unique_ptr<G4VisManager>(vis);
    }
    inline void SetUserInit(RMGHardware* det) { fDetectorConstruction = det; }
    inline void SetUserInit(G4VUserPhysicsList* proc) { fPhysicsList = proc; }
    inline void SetUserInit(RMGUserAction* ua) { fUserAction = ua; }
    inline void SetInteractive(bool flag = true) { fInteractive = flag; }
    inline void SetNumberOfThreads(int nthreads) { fNThreads = nthreads; }
    inline void SetPrintModulo(int n_ev) { fPrintModulo = n_ev > 0 ? n_ev : -1; }

    inline void EnablePersistency(bool flag = true) { fIsPersistencyEnabled = flag; }
    inline void IncludeMacroFile(std::string filename) { fMacroFileNames.emplace_back(filename); }
    void Initialize();
    void Run();

    void SetRandEngine(std::string name);
    void SetRandEngineSeed(long seed);
    void SetRandEngineInternalSeed(int index);
    void SetRandSystemEntropySeed();
    [[nodiscard]] inline bool GetRandIsControlled() const { return fIsRandControlled; }

    void SetLogLevel(std::string level);

    inline void SetOutputFileName(std::string filename) { fOutputFile = filename; }
    inline int RegisterNtuple(int det_uid) {
      auto res = fNtupleIDs.emplace(det_uid, fNtupleIDs.size());
      // RegisterNtuple will be called from different threads, with the same arguments.
      // Registering _new_ ntuples should only be possible from the main thread.
      if (!res.second && G4Threading::IsMasterThread())
        RMGLog::OutFormatDev(RMGLog::fatal, "Ntuple for detector with UID {} is already registered",
            det_uid);
      else if (res.second && !G4Threading::IsMasterThread())
        RMGLog::OutFormatDev(RMGLog::fatal, "Registering detector with UID {} from worker thread",
            det_uid);
      return this->GetNtupleID(det_uid);
    }
    inline int GetNtupleID(int det_uid) { return fNtupleIDs[det_uid]; }

  private:

    void SetUpDefaultG4RunManager(G4RunManagerType type = G4RunManagerType::Default);
    void SetUpDefaultG4VisManager();
    void SetUpDefaultDetectorConstruction();
    void SetUpDefaultProcessesList();
    void SetUpDefaultUserAction();

    std::unique_ptr<G4UIExecutive> StartInteractiveSession();

    std::string fApplicationName;
    int fArgc;
    char** fArgv;
    std::vector<std::string> fMacroFileNames;
    bool fIsRandControlled = false;
    bool fInteractive = false;
    bool fIsPersistencyEnabled = true;
    int fPrintModulo = -1;
    int fNThreads = 1;
    std::string fOutputFile = "detector-hits.root";
    // track internal id of detector NTuples
    std::map<int, int> fNtupleIDs;


    static RMGManager* fRMGManager;

    std::unique_ptr<G4RunManager> fG4RunManager = nullptr;
    std::unique_ptr<G4VisManager> fG4VisManager = nullptr;

    G4VUserPhysicsList* fPhysicsList = nullptr;
    RMGHardware* fDetectorConstruction = nullptr;
    RMGUserAction* fUserAction = nullptr;

    // messenger stuff
    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<G4GenericMessenger> fLogMessenger;
    std::unique_ptr<G4GenericMessenger> fRandMessenger;
    std::unique_ptr<G4GenericMessenger> fOutputMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

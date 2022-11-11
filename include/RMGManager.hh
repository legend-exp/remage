#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

#include <memory>
#include <vector>

#include "globals.hh"

#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisManager.hh"

#include "RMGLog.hh"

class G4VUserPhysicsList;
class RMGHardware;
class RMGUserAction;
class RMGManagerMessenger;
class G4GenericMessenger;
class RMGManager {

  public:

    RMGManager() = delete;
    RMGManager(std::string app_name, int argc, char** argv);
    ~RMGManager();

    RMGManager(RMGManager const&) = delete;
    RMGManager& operator=(RMGManager const&) = delete;
    RMGManager(RMGManager&&) = delete;
    RMGManager& operator=(RMGManager&&) = delete;

    // getters
    static inline RMGManager* GetRMGManager() { return fRMGManager; }
    G4RunManager* GetG4RunManager();
    G4VisManager* GetG4VisManager();
    RMGHardware* GetDetectorConstruction();
    G4VUserPhysicsList* GetProcessesList();
    inline int GetPrintModulo() { return fPrintModulo; }

    inline bool IsExecSequential() {
      return fG4RunManager->GetRunManagerType() == G4RunManager::RMType::sequentialRM;
    }
    inline bool IsPersistencyEnabled() { return fIsPersistencyEnabled; }
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
    inline void SetBatchMode(bool flag = true) { fBatchMode = flag; }
    inline void SetPrintModulo(int n_ev) { fPrintModulo = n_ev > 0 ? n_ev : -1; }

    inline void EnablePersistency(bool flag = true) { fIsPersistencyEnabled = flag; }
    inline void IncludeMacroFile(std::string filename) { fMacroFileNames.emplace_back(filename); }
    void Initialize();
    void Run();

    void SetRandEngine(std::string name);
    void SetRandEngineSeed(long seed);
    void SetRandEngineInternalSeed(int index);
    void SetRandSystemEntropySeed();
    inline bool GetRandIsControlled() { return fIsRandControlled; }

    void SetLogLevelScreen(std::string level);
    void SetLogLevelFile(std::string level);
    inline void SetLogToFileName(std::string filename) { RMGLog::OpenLogFile(filename); }

    inline void SetOutputFileName(std::string filename) { fOutputFile = filename; }

  private:

    void SetUpDefaultG4RunManager(G4RunManagerType type = G4RunManagerType::Default);
    void SetUpDefaultG4VisManager();
    void SetUpDefaultDetectorConstruction();
    void SetUpDefaultProcessesList();

    std::string fApplicationName;
    int fArgc;
    char** fArgv;
    std::vector<std::string> fMacroFileNames;
    bool fIsRandControlled = false;
    bool fBatchMode = false;
    bool fIsPersistencyEnabled = true;
    int fPrintModulo = -1;
    int fNThreads = 1;
    std::string fOutputFile = "detector-hits.root";

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

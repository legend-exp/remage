#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

#include <memory>
#include <vector>

#include "globals.hh"

#include "G4RunManager.hh"
#include "G4VisManager.hh"

#include "RMGLog.hh"

class G4VUserPhysicsList;
class RMGManagementDetectorConstruction;
class RMGManagementUserAction;
class RMGManagerMessenger;
class G4GenericMessenger;
class RMGManager {

  public:

    RMGManager() = delete;
    RMGManager(G4String app_name, int argc, char** argv);
    ~RMGManager();

    RMGManager           (RMGManager const&) = delete;
    RMGManager& operator=(RMGManager const&) = delete;
    RMGManager           (RMGManager&&)      = delete;
    RMGManager& operator=(RMGManager&&)      = delete;

    // getters
    static inline RMGManager*          GetRMGManager() { return fRMGManager; }
    G4RunManager*                      GetG4RunManager();
    G4VisManager*                      GetG4VisManager();
    RMGManagementDetectorConstruction* GetManagementDetectorConstruction();
    G4VUserPhysicsList*                GetRMGProcessesList();

    // setters
    inline void SetUserInitialization(G4RunManager* g4_manager) { fG4RunManager = std::unique_ptr<G4RunManager>(g4_manager); }
    inline void SetUserInitialization(G4VisManager* vis) { fG4VisManager = std::unique_ptr<G4VisManager>(vis); }
    inline void SetUserInitialization(RMGManagementDetectorConstruction* det) { fManagerDetectorConstruction = det; }
    inline void SetUserInitialization(G4VUserPhysicsList* proc) { fProcessesList = proc; }
    inline void SetBatchMode(G4bool flag=true) { fBatchMode = flag; }

    inline void IncludeMacroFile(G4String filename) { fMacroFileNames.emplace_back(filename); }
    void Initialize();
    void Run();

    void SetRandEngine(G4String name);
    void SetRandEngineSeed(G4long seed);
    void SetRandEngineInternalSeed(G4int index);
    void SetRandSystemEntropySeed();
    inline G4bool GetRandIsControlled() { return fIsRandControlled; }

    void SetLogLevelScreen(G4String level);
    void SetLogLevelFile(G4String level);
    inline void SetLogToFileName(G4String filename) { RMGLog::OpenLogFile(filename); }

  private:

    void SetupDefaultG4RunManager();
    void SetupDefaultG4VisManager();
    void SetupDefaultManagementDetectorConstruction();
    void SetupDefaultRMGProcessesList();

    G4String fApplicationName;
    int fArgc; char** fArgv;
    std::vector<G4String> fMacroFileNames;
    G4bool fIsRandControlled;
    G4bool fBatchMode;

    static RMGManager* fRMGManager;
    std::unique_ptr<G4RunManager> fG4RunManager;
    std::unique_ptr<G4VisManager> fG4VisManager;

    G4VUserPhysicsList* fProcessesList;
    RMGManagementDetectorConstruction*  fManagerDetectorConstruction;
    RMGManagementUserAction* fManagementUserAction;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<G4GenericMessenger> fLogMessenger;
    std::unique_ptr<G4GenericMessenger> fRandMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

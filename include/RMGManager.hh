#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

#include <memory>
#include <vector>

#include "globals.hh"

#include "G4RunManager.hh"
#include "G4VisManager.hh"

#include "RMGLog.hh"

class G4VUserPhysicsList;
class RMGDetectorConstruction;
class RMGUserAction;
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
    static inline RMGManager* GetRMGManager() { return fRMGManager; }
    G4RunManager* GetG4RunManager();
    G4VisManager* GetG4VisManager();
    RMGDetectorConstruction* GetDetectorConstruction();
    G4VUserPhysicsList* GetProcessesList();
    inline G4int GetPrintModulo() { return fPrintModulo; }

    // setters
    inline void SetUserInit(G4RunManager* g4_manager) { fG4RunManager = std::unique_ptr<G4RunManager>(g4_manager); }
    inline void SetUserInit(G4VisManager* vis) { fG4VisManager = std::unique_ptr<G4VisManager>(vis); }
    inline void SetUserInit(RMGDetectorConstruction* det) { fDetectorConstruction = det; }
    inline void SetUserInit(G4VUserPhysicsList* proc) { fProcessesList = proc; }
    inline void SetBatchMode(G4bool flag=true) { fBatchMode = flag; }
    inline void SetPrintModulo(G4int n_ev) { fPrintModulo = n_ev > 0 ? n_ev : -1; }

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

    void SetUpDefaultG4RunManager();
    void SetUpDefaultG4VisManager();
    void SetUpDefaultDetectorConstruction();
    void SetUpDefaultProcessesList();

    G4String fApplicationName;
    int fArgc; char** fArgv;
    std::vector<G4String> fMacroFileNames;
    G4bool fIsRandControlled;
    G4bool fBatchMode;
    G4int fPrintModulo;

    static RMGManager* fRMGManager;

    std::unique_ptr<G4RunManager> fG4RunManager;
    std::unique_ptr<G4VisManager> fG4VisManager;

    G4VUserPhysicsList*      fProcessesList;
    RMGDetectorConstruction* fDetectorConstruction;
    RMGUserAction*           fUserAction;

    // messenger stuff
    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<G4GenericMessenger> fLogMessenger;
    std::unique_ptr<G4GenericMessenger> fRandMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

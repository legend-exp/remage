#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

#include <memory>

#include "globals.hh"

#include "G4RunManager.hh"
#include "G4VisManager.hh"

class G4VUserPhysicsList;
class RMGManagementDetectorConstruction;
class RMGManagementUserAction;
class RMGManagerMessenger;
class RMGManager {

  public:

    RMGManager() = delete;
    RMGManager(G4String app_name);
    ~RMGManager();

    RMGManager           (RMGManager const&) = delete;
    RMGManager& operator=(RMGManager const&) = delete;
    RMGManager           (RMGManager&&)      = delete;
    RMGManager& operator=(RMGManager&&)      = delete;

    // getters
    static inline RMGManager*                 GetRMGManager() { return fRMGManager; }
    inline std::unique_ptr<G4RunManager>&     GetG4RunManager() { return fG4RunManager; }
    inline std::unique_ptr<G4VisManager>&     GetG4VisManager() { return fG4VisManager; }
    inline RMGManagementDetectorConstruction* GetManagementDetectorConstruction() { return fManagerDetectorConstruction; }
    inline G4VUserPhysicsList*                GetRMGProcessesList() { return fProcessesList; }

    // setters
    inline void SetUserInitialization(G4RunManager* g4_manager) { fG4RunManager = std::unique_ptr<G4RunManager>(g4_manager); }
    inline void SetUserInitialization(G4VisManager* vis) { fG4VisManager = std::unique_ptr<G4VisManager>(vis); }
    inline void SetUserInitialization(RMGManagementDetectorConstruction* det) { fManagerDetectorConstruction = det; }
    inline void SetUserInitialization(G4VUserPhysicsList* proc) { fProcessesList = proc; }

    G4bool ParseCommandLineArgs(int argc, char** argv);
    void PrintUsage();
    void Initialize();
    void Run();

    inline void SetControlledRandomization() { fControlledRandomization = true; }
    inline G4bool GetControlledRandomization() { return fControlledRandomization; }

  private:

    G4String fApplicationName;
    G4String fMacroFileName;
    G4bool   fControlledRandomization;

    static RMGManager* fRMGManager;
    std::unique_ptr<G4RunManager> fG4RunManager;
    std::unique_ptr<G4VisManager> fG4VisManager;

    G4VUserPhysicsList* fProcessesList;
    RMGManagementDetectorConstruction*  fManagerDetectorConstruction;
    RMGManagementUserAction* fManagementUserAction;

    std::unique_ptr<RMGManagerMessenger> fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

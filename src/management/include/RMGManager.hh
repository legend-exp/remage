#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

#include <memory>

#include "globals.hh"

#include "G4RunManager.hh"
#include "G4VisManager.hh"

class G4VUserPhysicsList;
class RMGManagerDetectorConstruction;
class RMGManagementUserActionInitialization;
class RMGGeneratorPrimary;
class RMGManagementEventAction;
class RMGManagementRunAction;
class RMGManagementSteppingAction;
class RMGManagementTrackingAction;
class RMGManagementStackingAction;
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
    static inline RMGManager*              GetRMGManager() { return fRMGManager; }
    inline std::unique_ptr<G4RunManager>&  GetG4RunManager() { return fG4RunManager; }
    inline std::unique_ptr<G4VisManager>&  GetG4VisManager() { return fG4VisManager; }
    inline RMGGeneratorPrimary*            GetRMGGeneratorPrimary() { return fGeneratorPrimary; }
    inline RMGManagerDetectorConstruction* GetManagerDetectorConstruction() { return fManagerDetectorConstruction; }
    // inline RMGManagementEventAction*       GetRMGEventAction() { return fManagementEventAction;}
    // inline RMGManagementRunAction*         GetRMGRunAction() { return fManagementRunAction;}
    // inline RMGManagementSteppingAction*    GetRMGSteppingAction() { return fManagementSteppingAction; }
    inline G4VUserPhysicsList*             GetRMGProcessesList() { return fProcessesList; }
    // inline RMGManagementStackingAction*    GetRMGStackingAction() { return fManagementStackingAction; }
    // inline RMGManagementTrackingAction*    GetRMGTrackingAction() { return fManagementTrackingAction; }

    // setters
    inline void SetUserInitialization(G4RunManager* g4_manager)            { fG4RunManager = std::unique_ptr<G4RunManager>(g4_manager); }
    inline void SetUserInitialization(G4VisManager* vis)                   { fG4VisManager = std::unique_ptr<G4VisManager>(vis); }
    inline void SetUserInitialization(RMGGeneratorPrimary* gene)           { fGeneratorPrimary = gene; }
    inline void SetUserInitialization(RMGManagerDetectorConstruction* det) { fManagerDetectorConstruction = det; }
    // inline void SetUserInitialization(RMGManagementEventAction* evt)       { fManagementEventAction = evt; }
    // inline void SetUserInitialization(RMGManagementRunAction* runa)        { fManagementRunAction = runa; }
    // inline void SetUserInitialization(RMGManagementTrackingAction* tracka) { fManagementTrackingAction = tracka; }
    // inline void SetUserInitialization(RMGManagementStackingAction* stacka) { fManagementStackingAction = stacka; }
    // inline void SetUserInitialization(RMGManagementSteppingAction* stepa)  { fManagementSteppingAction = stepa; }
    inline void SetUserInitialization(G4VUserPhysicsList* proc)            { fProcessesList = proc; }

    G4bool ParseCommandLineArgs(int argc, char** argv);
    void PrintUsage();
    void Initialize();
    void Run();

  private:

    G4String fApplicationName;

    static RMGManager* fRMGManager;
    std::unique_ptr<G4RunManager> fG4RunManager;
    std::unique_ptr<G4VisManager> fG4VisManager;

    G4VUserPhysicsList*              fProcessesList;
    RMGManagerDetectorConstruction*  fManagerDetectorConstruction;
    RMGGeneratorPrimary*             fGeneratorPrimary;
    RMGManagementUserActionInitialization* fManagementUserActionInitialization;

    // RMGManagementEventAction*        fManagementEventAction;
    // RMGManagementRunAction*          fManagementRunAction;
    // RMGManagementSteppingAction*     fManagementSteppingAction;
    // RMGManagementTrackingAction*     fManagementTrackingAction;
    // RMGManagementStackingAction*     fManagementStackingAction;

    G4String fMacroFileName;

    RMGManagerMessenger*             fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

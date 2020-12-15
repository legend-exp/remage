#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

class G4RunManager;
class G4VisManager;
class G4VUserPhysicsList;
class RMGGeneratorPrimary;
class RMGManagerDetectorConstruction;
class RMGManagementEventAction;
class RMGManagementRunAction;
class RMGManagementSteppingAction;
class RMGManagementTrackingAction;
class RMGManagementStackingAction;
class RMGManagerMessenger;
class RMGManager {

  public:

    RMGManager();
    ~RMGManager();

    RMGManager           (RMGManager const&) = delete;
    RMGManager& operator=(RMGManager const&) = delete;
    RMGManager           (RMGManager&&)      = delete;
    RMGManager& operator=(RMGManager&&)      = delete;

    // getters
    static inline RMGManager*              GetRMGManager() { return fRMGManager; }
    inline G4RunManager*                   GetG4RunManager() { return fG4RunManager; }
    inline G4VisManager*                   GetG4VisManager() { return fG4VisManager; }
    inline RMGGeneratorPrimary*            GetRMGGeneratorPrimary() { return fGeneratorPrimary; }
    inline RMGManagerDetectorConstruction* GetManagerDetectorConstruction() { return fManagerDetectorConstruction; }
    inline RMGManagementEventAction*       GetRMGEventAction() { return fManagementEventAction;}
    inline RMGManagementRunAction*         GetRMGRunAction() { return fManagementRunAction;}
    inline RMGManagementSteppingAction*    GetRMGSteppingAction() { return fManagementSteppingAction; }
    inline G4VUserPhysicsList*             GetRMGProcessesList() { return fProcessesList; }
    inline RMGManagementStackingAction*    GetRMGStackingAction() { return fManagementStackingAction; }
    inline RMGManagementTrackingAction*    GetRMGTrackingAction() { return fManagementTrackingAction; }

    // setters
    inline void SetUserInitialization(G4RunManager* g4_manager) { fG4RunManager = g4_manager; }
    inline void SetUserInitialization(G4VisManager* vis) { fG4VisManager = vis; }
    inline void SetUserInitialization(RMGGeneratorPrimary* gene) { fGeneratorPrimary = gene; }
    inline void SetUserInitialization(RMGManagerDetectorConstruction* det) { fManagerDetectorConstruction = det; }
    inline void SetUserInitialization(RMGManagementEventAction* evt) { fManagementEventAction = evt; }
    inline void SetUserInitialization(RMGManagementRunAction* runa) { fManagementRunAction = runa; }
    inline void SetUserInitialization(RMGManagementTrackingAction* tracka) { fManagementTrackingAction = tracka; }
    inline void SetUserInitialization(RMGManagementStackingAction* stacka) { fManagementStackingAction = stacka; }
    inline void SetUserInitialization(RMGManagementSteppingAction* stepa) { fManagementSteppingAction = stepa; }
    inline void SetUserInitialization(G4VUserPhysicsList* proc) { fProcessesList = proc; }

  private:

    G4RunManager*                    fG4RunManager;
    G4VisManager*                    fG4VisManager;
    G4VUserPhysicsList*              fProcessesList;
    RMGGeneratorPrimary*             fGeneratorPrimary;
    RMGManagerDetectorConstruction*  fManagerDetectorConstruction;
    RMGManagementEventAction*        fManagementEventAction;
    RMGManagementRunAction*          fManagementRunAction;
    RMGManagementSteppingAction*     fManagementSteppingAction;
    RMGManagementTrackingAction*     fManagementTrackingAction;
    RMGManagementStackingAction*     fManagementStackingAction;
    RMGManagerMessenger*             fG4Messenger;

    static RMGManager*               fRMGManager;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

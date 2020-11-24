#ifndef _MGMANAGER_HH_
#define _MGMANAGER_HH_

class G4RunManager;
class G4VisManager;
class G4VUserPhysicsList;
class MGGeneratorPrimary;
class MGManagerDetectorConstruction;
class MGManagementEventAction;
class MGManagementRunAction;
class MGManagementSteppingAction;
class MGManagementTrackingAction;
class MGManagementStackingAction;
class MGManagerMessenger;
class MGManager {

  public:

    MGManager();
    ~MGManager();

    MGManager           (MGManager const&) = delete;
    MGManager& operator=(MGManager const&) = delete;
    MGManager           (MGManager&&)      = delete;
    MGManager& operator=(MGManager&&)      = delete;

    // getters
    static inline MGManager*              GetMGManager() { return fMGManager; }
    inline G4RunManager*                  GetG4RunManager() { return fG4RunManager; }
    inline G4VisManager*                  GetG4VisManager() { return fG4VisManager; }
    inline MGGeneratorPrimary*            GetMGGeneratorPrimary() { return fGeneratorPrimary; }
    inline MGManagerDetectorConstruction* GetManagerDetectorConstruction() { return fManagerDetectorConstruction; }
    inline MGManagementEventAction*       GetMGEventAction() { return fManagementEventAction;}
    inline MGManagementRunAction*         GetMGRunAction() { return fManagementRunAction;}
    inline MGManagementSteppingAction*    GetMGSteppingAction() { return fManagementSteppingAction; }
    inline G4VUserPhysicsList*            GetMGProcessesList() { return fProcessesList; }
    inline MGManagementStackingAction*    GetMGStackingAction() { return fManagementStackingAction; }
    inline MGManagementTrackingAction*    GetMGTrackingAction() { return fManagementTrackingAction; }

    // setters
    inline void SetUserInitialization(G4RunManager* g4_manager) { fG4RunManager = g4_manager; }
    inline void SetUserInitialization(G4VisManager* vis) { fG4VisManager = vis; }
    inline void SetUserInitialization(MGGeneratorPrimary* gene) { fGeneratorPrimary = gene; }
    inline void SetUserInitialization(MGManagerDetectorConstruction* det) { fManagerDetectorConstruction = det; }
    inline void SetUserInitialization(MGManagementEventAction* evt) { fManagementEventAction = evt; }
    inline void SetUserInitialization(MGManagementRunAction* runa) { fManagementRunAction = runa; }
    inline void SetUserInitialization(MGManagementTrackingAction* tracka) {fManagementTrackingAction = tracka;}
    inline void SetUserInitialization(MGManagementStackingAction* stacka) {fManagementStackingAction = stacka;}
    inline void SetUserInitialization(MGManagementSteppingAction* stepa) { fManagementSteppingAction = stepa; }
    inline void SetUserInitialization(G4VUserPhysicsList* proc) { fProcessesList = proc; }

  private:

    G4RunManager*                   fG4RunManager;
    G4VisManager*                   fG4VisManager;
    G4VUserPhysicsList*             fProcessesList;
    MGGeneratorPrimary*             fGeneratorPrimary;
    MGManagerDetectorConstruction*  fManagerDetectorConstruction;
    MGManagementEventAction*        fManagementEventAction;
    MGManagementRunAction*          fManagementRunAction;
    MGManagementSteppingAction*     fManagementSteppingAction;
    MGManagementTrackingAction*     fManagementTrackingAction;
    MGManagementStackingAction*     fManagementStackingAction;
    MGManagerMessenger*             fG4Messenger;

    static MGManager*               fMGManager;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

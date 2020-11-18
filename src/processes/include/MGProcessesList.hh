#ifndef _MGPROCESSESLIST_HH
#define _MGPROCESSESLIST_HH

#include <map>

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class MGProcessesMessenger;
class MGProcessesList : public G4VModularPhysicsList {

  public:

    MGProcessesList();
    ~MGProcessesList() override;

    MGProcessesList           (MGProcessesList const&) = delete;
    MGProcessesList& operator=(MGProcessesList const&) = delete;
    MGProcessesList           (MGProcessesList&&)      = delete;
    MGProcessesList& operator=(MGProcessesList&&)      = delete;

    // setters
    void         SetCuts() override;
    void         SetRealm             (G4String realm_name);
    void         SetUseAngCorr        (G4int max_two_j);
    void         SetStoreICLevelData  (G4bool);
    inline void  SetOpticalFlag       (G4bool val) {fConstructOptical = val;};
    inline void  SetOpticalPhysicsOnly(G4bool val) {fUseOpticalPhysOnly = val;}
    inline void  SetLowEnergyFlag     (G4bool val) {fUseLowEnergy = val;};
    inline void  SetLowEnergyOption   (G4int  val) {fUseLowEnergyOption = val;};

    // getters
    void GetStepLimits();
    inline G4bool GetOpticalFlag() {return fConstructOptical;};

    void DumpPhysicsList();
    inline void LimitStepForParticle(G4String particle_name) {fLimitSteps[particle_name] = true;}

  protected:

    void ConstructParticle() override;
    void ConstructProcess() override;

    void AddTransportation();
    void AddParallelWorldScoring();
    void ConstructOp();
    void ConstructCerenkov();

  private:

    G4double fCutForOpticalPhoton;
    G4double fCutForGamma;
    G4double fCutForElectron;
    G4double fCutForPositron;
    G4double fCutForProton;
    G4double fCutForAlpha;
    G4double fCutForGenericIon;
    G4double fCutForGammaSensitive;
    G4double fCutForElectronSensitive;
    G4double fCutForPositronSensitive;

    G4bool fUseLowEnergy;
    G4int  fUseLowEnergyOption;
    G4bool fConstructOptical;
    G4bool fUseOpticalPhysOnly;

    G4String fPhysicsListHadrons;
    std::map<G4String, G4bool> fLimitSteps;
    MGProcessesMessenger* fProcessesMessenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMG_PROCESSES_LIST_HH_
#define _RMG_PROCESSES_LIST_HH_

#include <map>
#include <memory>

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class RMGProcessesMessenger;
class RMGProcessesList : public G4VModularPhysicsList {

  public:

    RMGProcessesList();

    RMGProcessesList           (RMGProcessesList const&) = delete;
    RMGProcessesList& operator=(RMGProcessesList const&) = delete;
    RMGProcessesList           (RMGProcessesList&&)      = delete;
    RMGProcessesList& operator=(RMGProcessesList&&)      = delete;

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
    inline void LimitStepForParticle(G4String particle_name) {fLimitSteps.at(particle_name) = true;}

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
    std::unique_ptr<RMGProcessesMessenger> fProcessesMessenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

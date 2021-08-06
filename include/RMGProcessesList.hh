#ifndef _RMG_PROCESSES_LIST_HH_
#define _RMG_PROCESSES_LIST_HH_

#include <map>
#include <memory>

#include "G4VModularPhysicsList.hh"
#include "G4GenericMessenger.hh"
#include "globals.hh"

class RMGProcessesMessenger;
class RMGProcessesList : public G4VModularPhysicsList {

  public:

    RMGProcessesList();

    RMGProcessesList           (RMGProcessesList const&) = delete;
    RMGProcessesList& operator=(RMGProcessesList const&) = delete;
    RMGProcessesList           (RMGProcessesList&&)      = delete;
    RMGProcessesList& operator=(RMGProcessesList&&)      = delete;

    enum PhysicsRealm {
      kDoubleBetaDecay,
      kDarkMatter,
      kCosmicRays,
      kLArScintillation
    };

    enum LowEnergyEMOption {
      kOption1,
      kOption2,
      kOption3,
      kOption4,
      kPenelope,
      kLivermore,
      kLivermorePolarized
    };

    // TODO: cut for optical photon?
    struct StepCutStore {
      StepCutStore() = default;
      inline StepCutStore(G4double def_cut) :
        gamma(def_cut), electron(def_cut), positron(def_cut),
        proton(def_cut), alpha(def_cut), generic_ion(def_cut) {}

      G4double gamma;
      G4double electron;
      G4double positron;
      G4double proton;
      G4double alpha;
      G4double generic_ion;
    };

    void SetCuts() override;
    void SetPhysicsRealm(PhysicsRealm realm);
    void SetPhysicsRealmString(G4String realm);
    void SetLowEnergyEMOptionString(G4String option);
    void SetUseGammaAngCorr(G4bool);
    void SetGammaTwoJMAX(G4int two_j_max);
    void SetStoreICLevelData(G4bool);

    inline void LimitStepForParticle(G4String particle_name) {fLimitSteps.at(particle_name) = true;}

  protected:

    void ConstructParticle() override;
    void ConstructProcess() override;

    void AddTransportation();
    void AddParallelWorldScoring();
    void ConstructOptical();
    void ConstructCerenkov();

  private:

    PhysicsRealm fPhysicsRealm;
    StepCutStore fStepCuts;
    StepCutStore fStepCutsSensitive;
    G4bool fConstructOptical;
    G4bool fUseLowEnergyEM;
    LowEnergyEMOption fLowEnergyEMOption;
    std::map<G4String, G4bool> fLimitSteps;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

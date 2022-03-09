#ifndef _RMG_PROCESSES_LIST_HH_
#define _RMG_PROCESSES_LIST_HH_

#include <map>
#include <memory>

#include "G4VModularPhysicsList.hh"
#include "G4GenericMessenger.hh"
#include "globals.hh"

class RMGProcessesMessenger;
class RMGPhysics : public G4VModularPhysicsList {

  public:

    RMGPhysics();

    RMGPhysics           (RMGPhysics const&) = delete;
    RMGPhysics& operator=(RMGPhysics const&) = delete;
    RMGPhysics           (RMGPhysics&&)      = delete;
    RMGPhysics& operator=(RMGPhysics&&)      = delete;

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
      inline StepCutStore(double def_cut) :
        gamma(def_cut), electron(def_cut), positron(def_cut),
        proton(def_cut), alpha(def_cut), generic_ion(def_cut) {}

      double gamma;
      double electron;
      double positron;
      double proton;
      double alpha;
      double generic_ion;
    };

    void SetCuts() override;
    void SetPhysicsRealm(PhysicsRealm realm);
    void SetPhysicsRealmString(std::string realm);
    void SetLowEnergyEMOptionString(std::string option);
    void SetUseGammaAngCorr(bool);
    void SetGammaTwoJMAX(int two_j_max);
    void SetStoreICLevelData(bool);

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
    bool fConstructOptical = false;
    bool fUseLowEnergyEM = true;
    LowEnergyEMOption fLowEnergyEMOption = LowEnergyEMOption::kLivermore;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

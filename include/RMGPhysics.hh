// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _RMG_PHYSICS_HH_
#define _RMG_PHYSICS_HH_

#include <map>
#include <memory>

#include "G4GenericMessenger.hh"
#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class RMGPhysics : public G4VModularPhysicsList {

  public:

    RMGPhysics();

    RMGPhysics(RMGPhysics const&) = delete;
    RMGPhysics& operator=(RMGPhysics const&) = delete;
    RMGPhysics(RMGPhysics&&) = delete;
    RMGPhysics& operator=(RMGPhysics&&) = delete;

    enum class PhysicsRealm {
      kDoubleBetaDecay,
      kDarkMatter,
      kCosmicRays,
      kLArScintillation
    };

    enum class LowEnergyEMOption {
      kOption1,
      kOption2,
      kOption3,
      kOption4,
      kPenelope,
      kLivermore,
      kLivermorePolarized,
      kNone
    };

    enum class HadronicPhysicsListOption {
      kQGSP_BIC_HP,
      kQGSP_BERT_HP,
      kFTFP_BERT_HP,
      kShielding,
      kNone
    };


    // TODO: cut for optical photon?
    struct StepCutStore {
        StepCutStore() = default;
        inline StepCutStore(double def_cut)
            : gamma(def_cut), electron(def_cut), positron(def_cut), proton(def_cut), alpha(def_cut),
              generic_ion(def_cut) {}

        double gamma;
        double electron;
        double positron;
        double proton;
        double alpha;
        double muon;
        double generic_ion;
    };

    void SetCuts() override;
    void SetPhysicsRealm(PhysicsRealm realm);
    void SetPhysicsRealmString(std::string realm);

    void SetLowEnergyEMOptionString(std::string option);
    void SetHadronicPhysicsListOptionString(std::string option);

    void SetUseThermalScattering(bool val) { fUseThermalScattering = val; }

    void SetUseGammaAngCorr(bool);
    void SetGammaTwoJMAX(int max_two_j);
    void SetStoreICLevelData(bool);

  protected:

    void ConstructParticle() override;
    void ConstructProcess() override;
    virtual void ConstructOptical();

  private:

    PhysicsRealm fPhysicsRealm = PhysicsRealm::kDoubleBetaDecay;
    StepCutStore fStepCuts = {};
    StepCutStore fStepCutsSensitive = {};
    bool fConstructOptical = false;
    bool fUseOpticalCustomWLS = false;
    bool fUseThermalScattering = false;
    bool fUseGrabmayrGammaCascades = false;
    LowEnergyEMOption fLowEnergyEMOption = LowEnergyEMOption::kLivermore;
    HadronicPhysicsListOption fHadronicPhysicsListOption = HadronicPhysicsListOption::kNone;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

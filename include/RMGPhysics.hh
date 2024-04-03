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

class RMGProcessesMessenger;
class RMGPhysics : public G4VModularPhysicsList {

  public:

    RMGPhysics();

    RMGPhysics(RMGPhysics const&) = delete;
    RMGPhysics& operator=(RMGPhysics const&) = delete;
    RMGPhysics(RMGPhysics&&) = delete;
    RMGPhysics& operator=(RMGPhysics&&) = delete;

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
    void SetUseGammaAngCorr(bool);
    void SetGammaTwoJMAX(int two_j_max);
    void SetStoreICLevelData(bool);

  protected:

    void ConstructParticle() override;
    void ConstructProcess() override;
    void ConstructOptical();

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

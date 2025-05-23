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

/** @brief Class to handle the physics lists, extends @c G4VModularPhysicsList
 *
 *  @details This handles selecting the physics list options (from those in geant4),
 *  and defining the stepping cuts and production cuts for the simulation.
 */
class RMGPhysics : public G4VModularPhysicsList {

  public:

    /** @brief Constructor for @c RMGPhysics , this sets the default choices.
     *
     * @details This sets the default prdduction cuts values. By default a cut
     * of 0.1 mm is used.
     */
    RMGPhysics();

    RMGPhysics(RMGPhysics const&) = delete;
    RMGPhysics& operator=(RMGPhysics const&) = delete;
    RMGPhysics(RMGPhysics&&) = delete;
    RMGPhysics& operator=(RMGPhysics&&) = delete;

    /** @brief Enum to specify a EM physics list from Geant4, see
     * [Geant4-manual](https://geant4.web.cern.ch/documentation/dev/plg_html/PhysicsListGuide/physicslistguide.html)
     * for more information */
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

    /** @brief Enum to specify a hardronic physics list from Geant4, see
     * [Geant4-manual](https://geant4.web.cern.ch/documentation/dev/plg_html/PhysicsListGuide/physicslistguide.html)
     * for more information */
    enum class HadronicPhysicsListOption {
      kQGSP_BIC_HP,
      kQGSP_BERT_HP,
      kFTFP_BERT_HP,
      kShielding,
      kNone
    };


    /** @brief Struct to hold the production cut values. */
    struct ProdCutStore {
        ProdCutStore() = default;

        /** @brief Constructor setting the default production cut @c def_cut */
        ProdCutStore(double def_cut)
            : gamma(def_cut), electron(def_cut), positron(def_cut), proton(def_cut), alpha(def_cut),
              generic_ion(def_cut) {}

        double gamma;
        double electron;
        double positron;
        double proton;
        double alpha;
        double generic_ion;
    };

    /** @brief Sets the production cut values, and energy range.
     *
     * @details  Defines a set of production cuts for the default region
     * and also for the G4Region "SensitiveRegion". This is created in RMGHardware, but this
     * behavior might be changed in a derived class, so careful here!
     */
    void SetCuts() override;

    /** @brief Sets the energy range for the production cut table*/
    void SetLowEnergyRange(G4double low_energy) { fLowEnergyRange = low_energy; };

    /** @brief Sets the energy range for the production cut table*/
    void SetHighEnergyRange(G4double high_energy) { fHighEnergyRange = high_energy; };

    /** @brief Set the production cut for the default region.
     *  @details The same cut is used for electrons, positrons and gammas.
     *  Notes:
     *   - No production cut alpha or generic ion is set.
     *  @param cut the production cut value for the default region.
     */
    void SetDefaultProductionCut(double cut);

    /** @brief Set the production cut for the sensitive region.
     *  @details The same cut is used for electrons, positrons and gammas.
     *  Notes:
     *   - No production cut alpha or generic ion is set.
     *  @param cut the production cut value for the sensitive region.
     */
    void SetSensitiveProductionCut(double cut);

    /** @brief Set the low energy EM options from a string, for use in the messenger. */
    void SetLowEnergyEMOptionString(std::string option);

    /** @brief Set the low energy EM options from a string, for use in the messenger. */
    void SetHadronicPhysicsListOptionString(std::string option);

    /** @brief Option to turn on thermal neutron scattering */
    void SetUseThermalScattering(bool val) { fUseThermalScattering = val; }

    /** @brief Option to turn on gamma emisson with correct angular correlations. */
    void SetUseGammaAngCorr(bool);

    void SetGammaTwoJMAX(int max_two_j);
    void SetStoreICLevelData(bool);


  protected:

    void ConstructParticle() override;
    void ConstructProcess() override;
    virtual void ConstructOptical();

  private:

    ProdCutStore fProdCuts = {};
    ProdCutStore fProdCutsSensitive = {};
    bool fConstructOptical = false;
    bool fUseOpticalCustomWLS = false;
    bool fUseThermalScattering = false;
    bool fUseGrabmayrGammaCascades = false;
    LowEnergyEMOption fLowEnergyEMOption = LowEnergyEMOption::kLivermore;
    HadronicPhysicsListOption fHadronicPhysicsListOption = HadronicPhysicsListOption::kNone;
    G4double fLowEnergyRange = 250 * CLHEP::eV;
    G4double fHighEnergyRange = 100. * CLHEP::GeV;
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

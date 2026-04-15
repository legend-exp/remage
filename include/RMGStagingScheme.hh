// Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#ifndef _RMG_STAGING_SCHEME_HH_
#define _RMG_STAGING_SCHEME_HH_

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

class G4Step;

/** @brief Centralized staging policy for waiting-stack based track deferral. */
class RMGStagingScheme : public RMGVOutputScheme {

  public:

    RMGStagingScheme();

    /** @brief Wraps @c G4UserStackingAction::StackingActionClassify
     *  @details This classifies configured optical photons and electrons as @c fWaiting.
     */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    /** @brief Evaluate optional stepping-time suspension criteria for configured particles. */
    void SteppingAction(const G4Step*) override;

    /** @brief Set the minimum distance to any other volume for an electron to be staged. */
    void SetElectronVolumeSafety(double safety) { fElectronVolumeSafety = safety; }

    /** @brief Add a volume name in which electron staging is active. */
    void AddElectronVolumeName(std::string volume) { fElectronVolumeNames.insert(volume); }

    /** @brief Set the maximum kinetic energy for e- tracks to be considered for staging.
     *  @details Only tracks with kinetic energy below this threshold will be staged.
     *  If set to a negative value, no energy threshold is applied.
     */
    void SetElectronMaxEnergyThresholdForStacking(double energy) {
      fElectronMaxEnergyThresholdForStacking = energy;
    }

    /** @brief Set the minimum distance to any other volume for a gamma to be staged. */
    void SetGammaVolumeSafety(double safety) { fGammaVolumeSafety = safety; }

    /** @brief Add a volume name in which gamma staging is active. */
    void AddGammaVolumeName(std::string volume) { fGammaVolumeNames.insert(volume); }

    /** @brief Set the maximum kinetic energy for gamma tracks to be considered for staging.
     *  @details Only tracks with kinetic energy below this threshold will be staged.
     *  If set to a negative value, no energy threshold is applied.
     */
    void SetGammaMaxEnergyThresholdForStacking(double energy) {
      fGammaMaxEnergyThresholdForStacking = energy;
    }

  private:

    std::unique_ptr<G4GenericMessenger> fElectronStagingMessengers;
    std::unique_ptr<G4GenericMessenger> fOpticalPhotonStagingMessengers;
    std::unique_ptr<G4GenericMessenger> fGammaStagingMessengers;


    void DefineCommands();

    std::optional<G4ClassificationOfNewTrack> Classify_OpticalPhoton(const G4Track* aTrack);
    std::optional<G4ClassificationOfNewTrack> Classify_Electron(const G4Track* aTrack);
    std::optional<G4ClassificationOfNewTrack> Classify_Gamma(const G4Track* aTrack);

    bool fDeferOpticalPhotonsToWaitingStage = false;

    bool fDeferElectronsToWaitingStage = false;
    double fElectronMaxEnergyThresholdForStacking = -1;
    bool fSuspendElectronsOnEnergyDrop = false;
    double fElectronVolumeSafety = -1;

    bool fDeferGammasToWaitingStage = false;
    double fGammaMaxEnergyThresholdForStacking = -1;
    bool fSuspendGammasOnEnergyDrop = false;
    double fGammaVolumeSafety = -1;

    std::set<std::string> fElectronVolumeNames;
    std::set<std::string> fGammaVolumeNames;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

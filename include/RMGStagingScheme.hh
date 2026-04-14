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

/** @brief Centralized staging policy for waiting-stack based track deferral. */
class RMGStagingScheme : public RMGVOutputScheme {

  public:

    RMGStagingScheme();

    /** @brief Wraps @c G4UserStackingAction::StackingActionClassify
     *  @details This classifies configured optical photons and electrons as @c fWaiting.
     */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    /** @brief Set the minimum distance to any other volume for an electron to be staged. */
    void SetElectronVolumeSafety(double safety) { fElectronVolumeSafety = safety; }

    /** @brief Add a volume name in which electron staging is active. */
    void AddElectronVolumeName(std::string volume) { fElectronVolumeNames.insert(volume); }

    /** @brief Enable or disable Germanium-only filtering for distance calculations.
     *  @details When enabled, surface distance checks only consider daughter volumes
     *  registered as Germanium detectors, potentially improving performance.
     */

    void SetDistanceCheckGermaniumOnly(bool enable);

    /** @brief Set the maximum kinetic energy for e- tracks to be considered for staging.
     *  @details Only tracks with kinetic energy below this threshold will be staged.
     *  If set to a negative value, no energy threshold is applied.
     */
    void SetElectronMaxEnergyThresholdForStacking(double energy) {
      fElectronMaxEnergyThresholdForStacking = energy;
    }

  private:

    std::unique_ptr<G4GenericMessenger> fElectronStagingMessengers;
    std::unique_ptr<G4GenericMessenger> fOpticalPhotonStagingMessengers;


    void DefineCommands();

    bool fDeferOpticalPhotonsToWaitingStage = false;

    bool fDeferElectronsToWaitingStage = false;
    double fElectronMaxEnergyThresholdForStacking = -1;
    double fElectronVolumeSafety = -1;

    std::set<std::string> fElectronVolumeNames;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

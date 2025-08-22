// Copyright (C) 2025 Eric Esch <https://orcid.org/0009-0000-4920-9313>
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

#ifndef _RMG_GENERAL_OUTPUT_SCHEME_HH_
#define _RMG_GENERAL_OUTPUT_SCHEME_HH_

#include <memory>
#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGDetectorHit.hh"
#include "RMGGeneralDetector.hh"
#include "RMGOutputTools.hh"
#include "RMGVOutputScheme.hh"

class G4Event;
/** @brief Output scheme for General detectors.
 *
 *  @details This output scheme records the hits in the General detectors.
 *  The properties of each @c RMGDetectorHit are recorded:
 *  - event index,
 *  - particle type,
 *  - time,
 *  - position,
 *  - energy deposition,
 *  - distance to detector surface.
 *
 * Optionally, the track ID and parent track ID can be stored as well as the
 * detector ID for the single table output mode.
 *
 * This OutputScheme is similar to the @c RMGGermaniumOutputScheme, but it offers
 * another label to apply e.g. different cuts and clustering than in the germanium
 */
class RMGGeneralOutputScheme : public RMGVOutputScheme {

  public:

    RMGGeneralOutputScheme();

    /** @brief Sets the names of the output columns, invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager* ana_man) override;

    /** @brief Store the information from the event, invoked in @c RMGEventAction::EndOfEventAction
     * @details Only steps with non-zero energy are stored, unless @c fDiscardZeroEnergyHits is false.
     */
    void StoreEvent(const G4Event* event) override;

    /** @brief Decide whether to store the event, invoked in @c RMGEventAction::EndOfEventAction
     *  @details @c true if the event should be discarded, else @c false .
     */
    bool ShouldDiscardEvent(const G4Event* event) override;

    /** @brief Wraps @c G4UserStackingAction::StackingActionNewStage
     *  @details discard all waiting events, if @c ShouldDiscardEvent() is true.
     */
    std::optional<bool> StackingActionNewStage(int) override;

    /** @brief Wraps @c G4UserStackingAction::StackingActionClassify
     *  @details This is used to classify all optical photon tracks as @c fWaiting if
     * @c fDiscardPhotonsIfNoGeneralEdep is true.
     */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    /** @brief Set a lower limit on the energy deposited in the event to store it. */
    void SetEdepCutLow(double threshold) { fEdepCutLow = threshold; }

    /** @brief Set a higher limit on the energy deposited in the event to store it. */
    void SetEdepCutHigh(double threshold) { fEdepCutHigh = threshold; }

    /** @brief Add a detector uid to the list of detectors to apply the energy cut for. */
    void AddEdepCutDetector(int det_uid) { fEdepCutDetectors.insert(det_uid); }

    /** @brief Set which position is used for the steps */
    void SetPositionMode(RMGOutputTools::PositionMode mode) { fPositionMode = mode; }

    /** @brief Set a distance to compute together steps in the bulk. */
    void SetClusterDistance(double threshold) { fPreClusterPars.cluster_distance = threshold; }

    /** @brief Set a distance to compute together steps in the surface */
    void SetClusterDistanceSurface(double threshold) {
      fPreClusterPars.cluster_distance_surface = threshold;
    }

    /** @brief Set the thickness of the surface region. */
    void SetSurfaceThickness(double thickness) { fPreClusterPars.surface_thickness = thickness; }

    /** @brief Set the time threshold for pre-clustering. */
    void SetClusterTimeThreshold(double threshold) {
      fPreClusterPars.cluster_time_threshold = threshold;
    }

    /** @brief Set the energy threshold to merge electron tracks.*/
    void SetElectronTrackEnergyThreshold(double threshold) {
      fPreClusterPars.track_energy_threshold = threshold;
    }

  protected:

    [[nodiscard]] std::string GetNtupleNameFlat() const override { return "general"; }

  private:

    RMGDetectorHitsCollection* GetHitColl(const G4Event*);
    void SetPositionModeString(std::string mode);

    std::vector<std::unique_ptr<G4GenericMessenger>> fMessengers;
    void DefineCommands();

    double fEdepCutLow = -1;
    double fEdepCutHigh = -1;
    std::set<int> fEdepCutDetectors;

    bool fDiscardPhotonsIfNoGeneralEdep = false;
    bool fDiscardZeroEnergyHits = true;

    bool fStoreSinglePrecisionEnergy = false;
    bool fStoreSinglePrecisionPosition = false;

    bool fStoreTrackID = false;
    bool fPreClusterHits = true;

    /** @brief Parameters for pre-clustering. */
    RMGOutputTools::ClusterPars fPreClusterPars{};

    // mode of position to store
    RMGOutputTools::PositionMode fPositionMode = RMGOutputTools::PositionMode::kAverage;

    std::map<std::string, G4ThreeVector> fDetectorOrigins;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

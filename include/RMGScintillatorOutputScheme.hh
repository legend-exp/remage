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

#ifndef _RMG_SCINTILLATOR_OUTPUT_SCHEME_HH_
#define _RMG_SCINTILLATOR_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGDetectorHit.hh"
#include "RMGOutputTools.hh"
#include "RMGScintillatorDetector.hh"
#include "RMGVOutputScheme.hh"

class G4Event;
/** @brief Output scheme for Scintillator detectors.
 *
 *  @details This output scheme records the hits in the Scintillator detectors.
 *  The properties of each @c RMGDetectorHit are recorded:
 *  - event index,
 *  - particle type,
 *  - time,
 *  - position,
 *  - energy deposition,
 *  - velocity of the particle (optional),
 *
 * Optionally, the track ID and parent track ID can be stored as well as the
 * detector ID for the single table output mode.
 *
 */
class RMGScintillatorOutputScheme : public RMGVOutputScheme {

  public:

    RMGScintillatorOutputScheme();

    /** @brief Sets the names of the output columns, invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager* ana_man) override;

    /** @brief Store the information from the event, invoked in @c RMGEventAction::EndOfEventAction
     * @details Only steps with non-zero energy are stored, unless @c fDiscardZeroEnergyHits is false.
     */
    void StoreEvent(const G4Event*) override;

    /** @brief Decide whether to store the event, invoked in @c RMGEventAction::EndOfEventAction
     *  @details @c true if the event should be discarded, else @c false .
     *  The event is discarded if there is no hit in the Scintillator volumes or the energy range
     *  condition is not met.
     */
    bool ShouldDiscardEvent(const G4Event*) override;

    /** @brief Set a lower cut on the energy deposited in the event to store it. */
    inline void SetEdepCutLow(double threshold) { fEdepCutLow = threshold; }

    /** @brief Set a upper cut on the energy deposited in the event to store it. */
    inline void SetEdepCutHigh(double threshold) { fEdepCutHigh = threshold; }

    /** @brief Add a detector uid to the list of detectors to apply the energy cut for. */
    inline void AddEdepCutDetector(int det_uid) { fEdepCutDetectors.insert(det_uid); }

    /** @brief Set which position is used for the steps */
    inline void SetPositionMode(RMGOutputTools::PositionMode mode) { fPositionMode = mode; }

    /** @brief Set a distance to compute together steps in the bulk. */
    inline void SetClusterDistance(double threshold) {
      fPreClusterPars.cluster_distance = threshold;
    }

    /** @brief Set the time threshold for pre-clustering. */
    inline void SetClusterTimeThreshold(double threshold) {
      fPreClusterPars.cluster_time_threshold = threshold;
    }

    /** @brief Set the energy threshold to merge electron tracks.*/
    inline void SetElectronTrackEnergyThreshold(double threshold) {
      fPreClusterPars.track_energy_threshold = threshold;
    }

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "scintillator"; }

  private:

    RMGDetectorHitsCollection* GetHitColl(const G4Event*);
    void SetPositionModeString(std::string mode);

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    double fEdepCutLow = -1;
    double fEdepCutHigh = -1;
    std::set<int> fEdepCutDetectors;

    bool fStoreSinglePrecisionEnergy = false;
    bool fStoreSinglePrecisionPosition = false;
    bool fStoreTrackID = false;

    bool fPreClusterHits = false;
    bool fDiscardZeroEnergyHits = true;

    /** @brief Parameters for pre-clustering. */
    RMGOutputTools::ClusterPars fPreClusterPars;

    /** @brief Mode of positions to store. */
    RMGOutputTools::PositionMode fPositionMode = RMGOutputTools::PositionMode::kAverage;

    bool fStoreVelocity = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

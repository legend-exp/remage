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

#ifndef _RMG_GERMANIUM_OUTPUT_SCHEME_HH_
#define _RMG_GERMANIUM_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGGermaniumDetector.hh"
#include "RMGVOutputScheme.hh"

class G4Event;
/** @brief Output scheme for Germanium detectors.
 *
 *  @details This output scheme records the hits in the Germanium detectors.
 *  The properties of each @c RMGGermaniumDetectorHit are recorded:
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
 * In addition, this class can be used for stacking tracks associated with optical
 * photons when no energy was deposited in Germanium.
 */
class RMGGermaniumOutputScheme : public RMGVOutputScheme {

  public:

    /** @brief Enum of which position of the hit to store. */
    enum class PositionMode {
      kPreStep,  /**Store the prestep point. */
      kPostStep, /**Store the poststep point. */
      kAverage,  /**Store the average. */
      kBoth,     /**Store both post and prestep */
    };

    RMGGermaniumOutputScheme();

    /** @brief Sets the names of the output columns, invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager* ana_man) override;

    /** @brief Store the information from the event, invoked in @c RMGEventAction::EndOfEventAction
     * @details Only steps with non-zero energy are stored, unless @c fDiscardZeroEnergyHits is false.
     */
    void StoreEvent(const G4Event* event) override;

    /** @brief Decide whether to store the event, invoked in @c RMGEventAction::EndOfEventAction
     *  @details @c true if the event should be discarded, else @c false .
     *  The event is discarded if there is no hit in Germanium or the energy range
     *  condition is not met.
     */
    bool ShouldDiscardEvent(const G4Event* event) override;

    /** @brief Perform a basic reduction of the hits collection removing very short steps.
     *
     *  @details This is based on a "within" track clustering (but note that some low energy tracks
     * can be merged by @c CombineLowEnergyTracks .
     * The steps in every track are looped through and combined into effective steps. A step is
     * added to the current cluster if:
     *
     * - it does not move from the surface region (defined by the @c
     * distance_to_surface<fSurfaceThickness) to the bulk or visa versa.
     * - the time difference between the step and the first of the cluster is not above @c
     * fClusterTimeThreshold ,
     * - the distance between the step and the first of the cluster is not above @c fClusterDistance
     * (for the bulk) or
     * @c fClusterSurfaceDistance for the surface.
     * - the hits in each cluster are then combined into one effective step with @c AverageHits
     *
     * @returns a collection of hits after pre-clustering.
     */
    RMGGermaniumDetectorHitsCollection* PreClusterHits(
        const RMGGermaniumDetectorHitsCollection* hits);

    /** @brief Average a cluster of hits to produce one effective hit.
     *
     * @details The steps in a cluster are average with the energy being the sum over the steps,
     * and the pre/post step position / distance to surface computed from the first/last step.
     * Other fields must be the same for all steps in the cluster and are taken from the first step.
     *
     * @returns the averaged hit.
     */
    RMGGermaniumDetectorHit* AverageHits(std::vector<RMGGermaniumDetectorHit*> hits);

    /** @brief Combine low energy electron tracks into their neighbours.
     *
     *  @details Some interactions of gammas, eg. Compton scattering or the
     * photoelectric effect can produce very low energy electron tracks. This function
     * reads a map of steps in each track (keyed by trackid), it then computes the
     * total energy in each electron track.
     *  If a track is below a certain threshold then the code searches through the
     * other tracks to see if there is one where the first pre-step point is
     * within the cluster distance of this track. If so they are combined for further
     * pre-clustering.
     *
     * @returns A map of steps after combining low energy tracks.
     */
    std::map<int, std::vector<RMGGermaniumDetectorHit*>> CombineLowEnergyElectronTracks(
        std::map<int, std::vector<RMGGermaniumDetectorHit*>> hits_map);

    /** @brief Wraps @c G4UserStackingAction::StackingActionNewStage
     *  @details discard all waiting events, if @c ShouldDiscardEvent() is true.
     */
    std::optional<bool> StackingActionNewStage(int) override;

    /** @brief Wraps @c G4UserStackingAction::StackingActionClassify
     *  @details This is used to classify all optical photon tracks as @c fWaiting if
     * @c fDiscardPhotonsIfNoGermaniumEdep is true.
     */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    /** @brief Set a lower cut on the energy deposited in the event to store it. */
    inline void SetEdepCutLow(double threshold) { fEdepCutLow = threshold; }

    /** @brief Set a lower cut on the energy deposited in the event to store it. */
    inline void SetEdepCutHigh(double threshold) { fEdepCutHigh = threshold; }

    /** @brief Add a detector uid to the list of detectors to apply the energy cut for. */
    inline void AddEdepCutDetector(int det_uid) { fEdepCutDetectors.insert(det_uid); }

    /** @brief Set which position is used for the steps */
    inline void SetPositionMode(PositionMode mode) { fPositionMode = mode; }

    /** @brief Set a distance to compute together steps in the bulk. */
    inline void SetClusterDistance(double threshold) { fClusterDistance = threshold; }

    /** @brief Set a distance to compute together steps in the surface */
    inline void SetClusterDistanceSurface(double threshold) { fClusterDistanceSurface = threshold; }

    /** @brief Set the thickness of the surface region. */
    inline void SetSurfaceThickness(double thickness) { fSurfaceThickness = thickness; }

    /** @brief Set the time threshold for pre-clustering. */
    inline void SetClusterTimeThreshold(double threshold) { fClusterTimeThreshold = threshold; }

    /** @brief Set the energy threshold to merge electron tracks.*/
    inline void SetElectronTrackEnergyThreshold(double threshold) {
      fTrackEnergyThreshold = threshold;
    }

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "germanium"; }

  private:

    RMGGermaniumDetectorHitsCollection* GetHitColl(const G4Event*);
    void SetPositionModeString(std::string mode);

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    double fEdepCutLow = -1;
    double fEdepCutHigh = -1;
    std::set<int> fEdepCutDetectors;

    bool fDiscardPhotonsIfNoGermaniumEdep = false;
    bool fDiscardZeroEnergyHits = true;

    bool fStoreSinglePrecisionEnergy = false;
    bool fStoreSinglePrecisionPosition = false;

    bool fStoreTrackID = false;
    bool fPreClusterHits = true;
    bool fCombineLowEnergyTracks = true;

    // clustering pars
    double fClusterTimeThreshold = 10 * CLHEP::us;
    double fClusterDistance = 10 * CLHEP::um;
    double fClusterDistanceSurface = 1 * CLHEP::um;
    double fSurfaceThickness = 2 * CLHEP::mm;
    double fTrackEnergyThreshold = 10 * CLHEP::keV;

    // mode of position to store
    PositionMode fPositionMode = PositionMode::kAverage;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

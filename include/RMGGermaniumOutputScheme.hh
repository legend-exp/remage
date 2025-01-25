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
 * In addition, this class is used for stacking tracks associated with optical
 * photons when no energy was deposited in Germanium.
 */
class RMGGermaniumOutputScheme : public RMGVOutputScheme {

  public:

    RMGGermaniumOutputScheme();

    /** @brief Sets the names of the output columns, invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager* ana_man) override;

    /** @brief Store the information from the event, invoked in @c RMGEventAction::EndOfEventAction
     */
    void StoreEvent(const G4Event* event) override;

    /** @brief Decide where to store the event, invoked in @c RMGEventAction::EndOfEventAction
     *  @details @c true if the event should be discarded, else @c false .
     *  The event is discard if there is not hit in Germanium of the energy range
     *  condition is not met.
     */
    bool ShouldDiscardEvent(const G4Event* event) override;

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

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "germanium"; }

  private:

    RMGGermaniumDetectorHitsCollection* GetHitColl(const G4Event*);

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    double fEdepCutLow = -1;
    double fEdepCutHigh = -1;
    std::set<int> fEdepCutDetectors;

    bool fDiscardPhotonsIfNoGermaniumEdep = false;

    bool fStoreSinglePrecisionEnergy = false;
    bool fStoreSinglePrecisionPosition = false;

    bool fStoreTrackID = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#ifndef _RMG_TRACK_OUTPUT_SCHEME_HH_
#define _RMG_TRACK_OUTPUT_SCHEME_HH_

#include <map>
#include <set>
#include <vector>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

class G4Event;
class G4Track;
/** @brief Output scheme for track information.
 *
 *  @details This output scheme records the properties of each track generated.
 *  The properties of each track recorded:
 *  - event index,
 *  - track ID,
 *  - parent track ID,
 *  - creator process,
 *  - particle type,
 *  - time,
 *  - position,
 *  - momentum,
 *  - kinetic energy
 *
 *  The creator process is mapped to a unique integer value and additionally stored in the output.
 *
 *  It can be specified that the information is always stored, even if the
 *  event would be discarded by other output schemes.
 */
class RMGTrackOutputScheme : public RMGVOutputScheme {

  public:

    RMGTrackOutputScheme();

    /** @brief Sets the names of the output columns, invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager*) override;

    /** @brief Called in @c RMGTrackingAction::PreUserTrackingAction to collect information
     *  about the track before it is processed.
     */
    void TrackingActionPre(const G4Track*) override;

    /** @brief handles the storage of the process map. */
    void EndOfRunAction(const G4Run*) override;

    /** @brief Clears the event data and frees memory before the next event is processed. */
    void ClearBeforeEvent() override;

    /** @brief Store the information from the event, invoked in @c RMGEventAction::EndOfEventAction
     * @details If @c fStoreAlways is true, the information is always stored, even if the event
     * would be discarded by other output schemes.
     */
    void StoreEvent(const G4Event*) override;

    /** @brief Sets @c fStoreAlways variable to decide if the information is always stored. */
    [[nodiscard]] bool StoreAlways() const override { return fStoreAlways; }

  protected:

    [[nodiscard]] std::string GetNtupleName(RMGDetectorMetadata) const override {
      throw std::logic_error("step output scheme has no detectors");
    }

    void AddParticleFilter(const int pdg) { fFilterParticle.insert(pdg); }
    void AddProcessFilter(const std::string proc) { fFilterProcess.insert(proc); }
    void SetEnergyFilter(double energy) { fFilterEnergy = energy; }

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    bool fStoreSinglePrecisionEnergy = false;
    bool fStoreSinglePrecisionPosition = false;
    bool fStoreAlways = false;

    std::map<std::string, uint32_t> fProcessMap;

    std::set<std::string> fFilterProcess;
    std::set<int> fFilterParticle;
    double fFilterEnergy = -1;

    struct RMGTrackEntry {
        int eventId;
        int trackId;
        int parentId;
        int procId;
        int particlePdg;
        double globalTime;
        double xPosition;
        double yPosition;
        double zPosition;
        double px;
        double py;
        double pz;
        double kineticEnergy;
    };

    std::vector<RMGTrackEntry> fTrackEntries;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

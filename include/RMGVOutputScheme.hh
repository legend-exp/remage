// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

#ifndef _RMG_V_OUTPUT_SCHEME_HH_
#define _RMG_V_OUTPUT_SCHEME_HH_

#include <optional>
#include <string>

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4Track.hh"
#include "G4UserStackingAction.hh"

#include "RMGDetectorMetadata.hh"

#include "fmt/format.h"

class G4Event;
/**
 * @brief Virtual output scheme interface.
 *
 * This abstract class defines the interface for output schemes that transform and store
 * hit-level simulation data into persistent storage (e.g. ntuples). It provides virtual methods
 * for setting output names, clearing event data, storing event information, and for hooking into
 * various Geant4 user actions (e.g. stacking, tracking, etc.).
 */
class RMGVOutputScheme {

  public:

    RMGVOutputScheme() = default;
    virtual ~RMGVOutputScheme() = default;

    // initialization.
    /**
     * @brief Initialize ntuple column names for this output scheme.
     *
     * This function is called during run initialization to create and assign
     * output column names for the analysis manager.
     */
    virtual void AssignOutputNames(G4AnalysisManager*) {}

    // functions for individual events.
    /**
     * @brief Clear any event-specific data.
     *
     * Called before processing a new event, this function should clear any stored data
     * from the previous event.
     */
    virtual void ClearBeforeEvent() {}
    /**
     * @brief Decide whether to discard the current event.
     *
     * This function is called at the end of each event. It can be used to filter out events
     * that do not meet specified criteria.
     *
     * @return True if the event should be discarded, false otherwise.
     */
    virtual bool ShouldDiscardEvent(const G4Event*) { return false; }
    /**
     * @brief Indicates whether the output scheme always stores event data.
     *
     * Useful for output schemes that should always write out information regardless
     * of filtering criteria defined by any output scheme in
     * @ref RMGVOutputScheme::ShouldDiscardEvent.
     *
     * @return True if the scheme always stores event data, false otherwise.
     */
    [[nodiscard]] virtual bool StoreAlways() const { return false; }
    /**
     * @brief Store the event data.
     *
     * This function is invoked at the end of an event to store the output data in the persistent
     * file. Derived classes should implement how event data is recorded.
     */
    virtual void StoreEvent(const G4Event*) {}

    // hook into RMGStackingAction.
    /**
     * @brief Hook for classifying new tracks during the stacking phase.
     *
     * This method allows the output scheme to classify tracks in the stacking action,
     * e.g. to discard or temporarily hold them.
     *
     * @c aTrack is the pointer to the @c G4Track being classified.  @c stage
     * is the current stage index in the stacking process.
     *
     * @return An optional classification value; if empty, no classification is applied.
     */
    virtual std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, const int) {
      return std::nullopt;
    }
    /**
     * @brief Hook for transitioning to a new stacking stage.
     *
     * Output schemes can use this method to determine whether waiting tracks should be cleared
     * as the stacking process advances to a new stage.
     *
     * @c stage is the new stacking stage index.
     *
     * @return An optional boolean decision; if empty, no action is taken.
     */
    virtual std::optional<bool> StackingActionNewStage(const int) { return std::nullopt; }

    // hook into G4TrackingAction.
    /**
     * @brief Hook called before tracking a new particle.
     *
     * Output schemes may use this to record any track-specific information required for output.
     */
    virtual void TrackingActionPre(const G4Track*) {};

    /**
     * @brief Perform final actions at the end of a run.
     *
     * This function can be used by derived output schemes to finalize or write remaining data.
     */
    virtual void EndOfRunAction(const G4Run*) {};

    // only to be called by the manager, before calling @ref AssignOutputNames.
    /**
     * @brief Specify whether to create separate ntuples for each detector.
     *
     * @param ntuple_per_det True to assign one ntuple per detector; false to use a single shared
     * ntuple per output scheme type.
     */
    void SetNtuplePerDetector(bool ntuple_per_det) { fNtuplePerDetector = ntuple_per_det; }
    /**
     * @brief Specify whether to use the physical volume name for naming ntuples.
     *
     * @param use_vol_name True to use the volume name; false to use the naming
     * scheme based on the detector uid.
     */
    void SetNtupleUseVolumeName(bool use_vol_name) { fNtupleUseVolumeName = use_vol_name; }

  protected:

    [[nodiscard]] virtual std::string GetNtupleName(RMGDetectorMetadata det) const {
      if (fNtuplePerDetector) {
        if (!det.name.empty() && fNtupleUseVolumeName) { return det.name; }
        return fmt::format("det{:03}", det.uid);
      }
      return GetNtupleNameFlat();
    }
    [[nodiscard]] virtual std::string GetNtupleNameFlat() const {
      throw new std::logic_error("GetNtupleNameFlat not implemented");
    }

    // helper functions for output schemes.
    void CreateNtupleFOrDColumn(G4AnalysisManager* ana_man, int nt, std::string name, bool use_float) {
      if (use_float) ana_man->CreateNtupleFColumn(nt, name);
      else ana_man->CreateNtupleDColumn(nt, name);
    }
    void FillNtupleFOrDColumn(G4AnalysisManager* ana_man, int nt, int col, double val, bool use_float) {
      if (use_float)
        ana_man->FillNtupleFColumn(nt, col, val); // NOLINT(cppcoreguidelines-narrowing-conversions)
      else ana_man->FillNtupleDColumn(nt, col, val);
    }

    // global options injected by manager.
    bool fNtuplePerDetector = true;
    bool fNtupleUseVolumeName = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

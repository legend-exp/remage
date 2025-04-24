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

#ifndef _MUG_GERMANIUM_DETECTOR_HH_
#define _MUG_GERMANIUM_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"

#include "RMGDetectorHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
/** @brief Class to describe the germanium detector, mainly handles processing of the detected hits.
 * Extends @c G4VSensitiveDetector */
class RMGGermaniumDetector : public G4VSensitiveDetector {

  public:

    RMGGermaniumDetector();
    ~RMGGermaniumDetector() = default;

    RMGGermaniumDetector(RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector(RMGGermaniumDetector&&) = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector&&) = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;

    /** @brief Process the detected hits computing the various parameters of a @c
     * RMGDetectorHit and then adding this to the hit collection.*/
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

    /** @brief Check if the step point is contained in a physical volume registered as a Germanium detector.
     *
     * @param step_point The step point (either post or pre step) to check.
     */
    bool CheckStepPointContainment(const G4StepPoint* step_point);

  private:

    RMGDetectorHitsCollection* fHitsCollection = nullptr;
};


#endif

// vim: tabstop=2 shiftwidth=2 expandtab

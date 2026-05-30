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
/**
 * @brief Sensitive detector producing @ref RMGDetectorHit instances for germanium volumes.
 *
 * In addition to energy deposition and step geometry, each hit carries the distance from
 * pre/post-step and average point to the surface of the sensitive volume — used downstream
 * by detector-response models. Hits are persisted by @ref RMGGermaniumOutputScheme.
 */
class RMGGermaniumDetector : public G4VSensitiveDetector {

  public:

    RMGGermaniumDetector();
    ~RMGGermaniumDetector() = default;

    RMGGermaniumDetector(RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector(RMGGermaniumDetector&&) = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector&&) = delete;

    /** @brief Allocate and register the hit collection for the current event. */
    void Initialize(G4HCofThisEvent* hit_coll) override;

    /** @brief Build an @ref RMGDetectorHit from @p step and add it to the hits collection. */
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGDetectorHitsCollection* fHitsCollection = nullptr;
};


#endif

// vim: tabstop=2 shiftwidth=2 expandtab

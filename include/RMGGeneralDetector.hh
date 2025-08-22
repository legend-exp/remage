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

#ifndef _RMG_GENERAL_DETECTOR_HH_
#define _RMG_GENERAL_DETECTOR_HH_

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
/** @brief Class to describe the general detector, mainly handles processing of the detected hits.
 * Extends @c G4VSensitiveDetector */
class RMGGeneralDetector : public G4VSensitiveDetector {

  public:

    RMGGeneralDetector();
    ~RMGGeneralDetector() = default;

    RMGGeneralDetector(RMGGeneralDetector const&) = delete;
    RMGGeneralDetector& operator=(RMGGeneralDetector const&) = delete;
    RMGGeneralDetector(RMGGeneralDetector&&) = delete;
    RMGGeneralDetector& operator=(RMGGeneralDetector&&) = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;

    /** @brief Process the detected hits computing the various parameters of a @c
     * RMGDetectorHit and then adding this to the hit collection.*/
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGDetectorHitsCollection* fHitsCollection = nullptr;
};


#endif

// vim: tabstop=2 shiftwidth=2 expandtab

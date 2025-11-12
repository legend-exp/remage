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

#ifndef _RMG_CALORIMETER_DETECTOR_HH_
#define _RMG_CALORIMETER_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4VSensitiveDetector.hh"

#include "RMGDetectorHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
/** @brief Class to describe the germanium detector, mainly handles processing of the detected hits.
 * Extends @c G4VSensitiveDetector */
class RMGCalorimeterDetector : public G4VSensitiveDetector {

  public:

    RMGCalorimeterDetector();
    ~RMGCalorimeterDetector() = default;

    RMGCalorimeterDetector(RMGCalorimeterDetector const&) = delete;
    RMGCalorimeterDetector& operator=(RMGCalorimeterDetector const&) = delete;
    RMGCalorimeterDetector(RMGCalorimeterDetector&&) = delete;
    RMGCalorimeterDetector& operator=(RMGCalorimeterDetector&&) = delete;

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

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

#ifndef _RMG_SCINTILLATOR_DETECTOR_HH_
#define _RMG_SCINTILLATOR_DETECTOR_HH_

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
/** @brief Class to describe the scintillator detector, mainly handles processing of the detected
 * hits. Extends @c G4VSensitiveDetector */
class RMGScintillatorDetector : public G4VSensitiveDetector {

  public:

    RMGScintillatorDetector();
    ~RMGScintillatorDetector() = default;

    RMGScintillatorDetector(RMGScintillatorDetector const&) = delete;
    RMGScintillatorDetector& operator=(RMGScintillatorDetector const&) = delete;
    RMGScintillatorDetector(RMGScintillatorDetector&&) = delete;
    RMGScintillatorDetector& operator=(RMGScintillatorDetector&&) = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGDetectorHitsCollection* fHitsCollection = nullptr;
};


#endif

// vim: tabstop=2 shiftwidth=2 expandtab

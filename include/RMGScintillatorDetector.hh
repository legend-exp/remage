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

#ifndef _RMG_SCINTILLATOR_DETECTOR_HH_
#define _RMG_SCINTILLATOR_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"

class RMGScintillatorDetectorHit : public G4VHit {

  public:

    RMGScintillatorDetectorHit() = default;
    ~RMGScintillatorDetectorHit() = default;

    RMGScintillatorDetectorHit(RMGScintillatorDetectorHit const&) = delete;
    RMGScintillatorDetectorHit& operator=(RMGScintillatorDetectorHit const&) = delete;
    RMGScintillatorDetectorHit(RMGScintillatorDetectorHit&&) = delete;
    RMGScintillatorDetectorHit& operator=(RMGScintillatorDetectorHit&&) = delete;

    bool operator==(const RMGScintillatorDetectorHit&) const;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    void Print() override;
    void Draw() override;

    int detector_uid = -1;
    int particle_type = -1;
    double energy_deposition = -1;
    G4ThreeVector global_position_pre;
    G4ThreeVector global_position_post;
    double global_time = -1;
    double velocity_pre = -1;
    double velocity_post = -1;
};

using RMGScintillatorDetectorHitsCollection = G4THitsCollection<RMGScintillatorDetectorHit>;

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
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

    RMGScintillatorDetectorHitsCollection* fHitsCollection = nullptr;
};

/// \cond this triggers a sphinx error
extern G4ThreadLocal G4Allocator<RMGScintillatorDetectorHit>* RMGScintillatorDetectorHitAllocator;
/// \endcond

inline void* RMGScintillatorDetectorHit::operator new(size_t) {
  if (!RMGScintillatorDetectorHitAllocator)
    RMGScintillatorDetectorHitAllocator = new G4Allocator<RMGScintillatorDetectorHit>;
  return (void*)RMGScintillatorDetectorHitAllocator->MallocSingle();
}

inline void RMGScintillatorDetectorHit::operator delete(void* hit) {
  RMGScintillatorDetectorHitAllocator->FreeSingle(static_cast<RMGScintillatorDetectorHit*>(hit));
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

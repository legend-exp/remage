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
#include "G4GenericMessenger.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"

class RMGGermaniumDetectorHit : public G4VHit {

  public:

    RMGGermaniumDetectorHit() = default;
    ~RMGGermaniumDetectorHit() = default;

    RMGGermaniumDetectorHit(RMGGermaniumDetectorHit const&) = delete;
    RMGGermaniumDetectorHit& operator=(RMGGermaniumDetectorHit const&) = delete;
    RMGGermaniumDetectorHit(RMGGermaniumDetectorHit&&) = delete;
    RMGGermaniumDetectorHit& operator=(RMGGermaniumDetectorHit&&) = delete;

    bool operator==(const RMGGermaniumDetectorHit&) const;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    void Print() override;
    void Draw() override;

    int detector_uid = -1;
    int particle_type = -1;
    float energy_deposition = -1;
    G4ThreeVector global_position;
    double global_time = -1;
};

using RMGGermaniumDetectorHitsCollection = G4THitsCollection<RMGGermaniumDetectorHit>;

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
class RMGGermaniumDetector : public G4VSensitiveDetector {

  public:

    RMGGermaniumDetector();
    ~RMGGermaniumDetector() = default;

    RMGGermaniumDetector(RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector(RMGGermaniumDetector&&) = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector&&) = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGGermaniumDetectorHitsCollection* fHitsCollection = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();
};

extern G4ThreadLocal G4Allocator<RMGGermaniumDetectorHit>* RMGGermaniumDetectorHitAllocator;

inline void* RMGGermaniumDetectorHit::operator new(size_t) {
  if (!RMGGermaniumDetectorHitAllocator)
    RMGGermaniumDetectorHitAllocator = new G4Allocator<RMGGermaniumDetectorHit>;
  return (void*)RMGGermaniumDetectorHitAllocator->MallocSingle();
}

inline void RMGGermaniumDetectorHit::operator delete(void* hit) {
  RMGGermaniumDetectorHitAllocator->FreeSingle((RMGGermaniumDetectorHit*)hit);
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

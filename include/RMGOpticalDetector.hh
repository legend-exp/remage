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

#ifndef _MUG_OPTICAL_DETECTOR_HH_
#define _MUG_OPTICAL_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4Allocator.hh"
#include "G4GenericMessenger.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4THitsCollection.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"

class RMGOpticalDetectorHit : public G4VHit {

  public:

    RMGOpticalDetectorHit() = default;
    ~RMGOpticalDetectorHit() = default;

    RMGOpticalDetectorHit(RMGOpticalDetectorHit const&) = delete;
    RMGOpticalDetectorHit& operator=(RMGOpticalDetectorHit const&) = delete;
    RMGOpticalDetectorHit(RMGOpticalDetectorHit&&) = delete;
    RMGOpticalDetectorHit& operator=(RMGOpticalDetectorHit&&) = delete;

    bool operator==(const RMGOpticalDetectorHit&) const;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    void Print() override;
    // TODO
    // void Draw() override;

    int detector_uid = -1;
    float photon_wavelength = 0.;
    double global_time = -1;
};

using RMGOpticalDetectorHitsCollection = G4THitsCollection<RMGOpticalDetectorHit>;

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
class RMGOpticalDetector : public G4VSensitiveDetector {

  public:

    RMGOpticalDetector();
    ~RMGOpticalDetector() = default;

    RMGOpticalDetector(RMGOpticalDetector const&) = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector const&) = delete;
    RMGOpticalDetector(RMGOpticalDetector&&) = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector&&) = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGOpticalDetectorHitsCollection* fHitsCollection = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    G4bool fUseQuantumEfficiency = false;
    void ReadDatasheet(G4String pathToDatasheet);
    G4PhysicsOrderedFreeVector* fQuantumEfficency = nullptr;
};

extern G4ThreadLocal G4Allocator<RMGOpticalDetectorHit>* RMGOpticalDetectorHitAllocator;

inline void* RMGOpticalDetectorHit::operator new(size_t) {
  if (!RMGOpticalDetectorHitAllocator)
    RMGOpticalDetectorHitAllocator = new G4Allocator<RMGOpticalDetectorHit>;
  return (void*)RMGOpticalDetectorHitAllocator->MallocSingle();
}

inline void RMGOpticalDetectorHit::operator delete(void* hit) {
  RMGOpticalDetectorHitAllocator->FreeSingle((RMGOpticalDetectorHit*)hit);
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

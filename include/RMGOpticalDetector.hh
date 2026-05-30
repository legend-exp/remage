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

#ifndef _MUG_OPTICAL_DETECTOR_HH_
#define _MUG_OPTICAL_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4TouchableHandle.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"

/**
 * @brief Hit produced by @ref RMGOpticalDetector when an optical photon is absorbed.
 *
 * Holds the detector identifier, the photon wavelength and the global absorption time.
 */
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
    /** @brief Color the detector volume if hit. */
    void Draw() override;

    G4TouchableHandle detector_touchable; ///< Touchable of the absorbing volume, used by @ref Draw.
    int detector_uid = -1;                ///< Remage unique identifier of the absorbing detector.
    double photon_wavelength = 0.;        ///< Absorbed-photon wavelength (Geant4 length units).
    double global_time = -1;              ///< Global time at absorption (Geant4 time units).
};

using RMGOpticalDetectorHitsCollection = G4THitsCollection<RMGOpticalDetectorHit>;

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
/**
 * @brief Sensitive detector for optical photon absorption.
 *
 * Emits one @ref RMGOpticalDetectorHit per absorbed @c opticalphoton track and kills the
 * track. Hits are persisted by the optical detector output scheme.
 */
class RMGOpticalDetector : public G4VSensitiveDetector {

  public:

    RMGOpticalDetector();
    ~RMGOpticalDetector() = default;

    RMGOpticalDetector(RMGOpticalDetector const&) = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector const&) = delete;
    RMGOpticalDetector(RMGOpticalDetector&&) = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector&&) = delete;

    /** @brief Allocate and register the hit collection for the current event. */
    void Initialize(G4HCofThisEvent* hit_coll) override;
    /** @brief Record an absorbed optical photon as a hit. */
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGOpticalDetectorHitsCollection* fHitsCollection = nullptr;
};

/// \cond this triggers a sphinx error
extern G4ThreadLocal G4Allocator<RMGOpticalDetectorHit>* RMGOpticalDetectorHitAllocator;
/// \endcond

inline void* RMGOpticalDetectorHit::operator new(size_t) {
  if (!RMGOpticalDetectorHitAllocator)
    RMGOpticalDetectorHitAllocator = new G4Allocator<RMGOpticalDetectorHit>;
  return (void*)RMGOpticalDetectorHitAllocator->MallocSingle();
}

inline void RMGOpticalDetectorHit::operator delete(void* hit) {
  RMGOpticalDetectorHitAllocator->FreeSingle(static_cast<RMGOpticalDetectorHit*>(hit));
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

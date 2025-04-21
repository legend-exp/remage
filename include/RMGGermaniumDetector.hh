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

/** @brief Class to store hits in the Germanium detectors, extends @c G4VHit */
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

    double energy_deposition = -1;
    double distance_to_surface_prestep = -1;
    double distance_to_surface_average = -1;
    double distance_to_surface_poststep = -1;

    G4ThreeVector global_position_poststep;
    G4ThreeVector global_position_prestep;
    G4ThreeVector global_position_average;

    double global_time = -1;
    int track_id = -1;
    int parent_track_id = -1;

    G4VPhysicalVolume* physical_volume = nullptr;
};

using RMGGermaniumDetectorHitsCollection = G4THitsCollection<RMGGermaniumDetectorHit>;


/** @brief Class to describe the germanium detector, mainly handles processing of the detected hits.
 * Extends @c G4VSensitiveDetector */
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

    /** @brief Process the detected hits computing the various parameters of a @c
     * RMGGermaniumDetectorHit and then adding this to the hit collection.*/
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

    /** @brief Compute the distance from the point to the surface of the physical volume.
     * @details Checks distance to surfaces of mother volume.
     * @param pv The physical volume to find the distance to.
     * @param position The position to evaluate the distance for.
     */
    static double DistanceToSurface(const G4VPhysicalVolume* pv, const G4ThreeVector& position);

    /** @brief Check if the step point is contained in a physical volume registered as a Germanium detector.
     *
     * @param step_point The step point (either post or pre step) to check.
     */
    bool CheckStepPointContainment(const G4StepPoint* step_point);

  private:

    RMGGermaniumDetectorHitsCollection* fHitsCollection = nullptr;
};

/// \cond this triggers a sphinx error
extern G4ThreadLocal G4Allocator<RMGGermaniumDetectorHit>* RMGGermaniumDetectorHitAllocator;
/// \endcond

inline void* RMGGermaniumDetectorHit::operator new(size_t) {
  if (!RMGGermaniumDetectorHitAllocator)
    RMGGermaniumDetectorHitAllocator = new G4Allocator<RMGGermaniumDetectorHit>;
  return (void*)RMGGermaniumDetectorHitAllocator->MallocSingle();
}

inline void RMGGermaniumDetectorHit::operator delete(void* hit) {
  RMGGermaniumDetectorHitAllocator->FreeSingle(static_cast<RMGGermaniumDetectorHit*>(hit));
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

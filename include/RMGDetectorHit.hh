// Copyright (C) 2025 Toby Dixon <https://orcid.org/0000-0001-8787-6336>
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


#ifndef _RMG_DETECTOR_HIT_HH
#define _RMG_DETECTOR_HIT_HH

#include "G4Colour.hh"
#include "G4THitsCollection.hh"
#include "G4VHit.hh"
#include "G4VPhysicalVolume.hh"


/** @brief Class to store hits in the Germanium or Scintillator detectors, extends @c G4VHit
 *
 * @details Stores the information on a given hit in the detector:
 * - detector ID,
 * - particle type,
 * - energy deposited,
 * - position of the step (both pre and post-step),
 * - time,
 * - track id and parent track id,
 * - distance of the step from the sensitive detector surface,
 * - pointer to the @c G4PhysicalVolume the step took place in.
 * This information can then be saved by the @ref RMGGermaniumOutputScheme , or
 * @ref RMGScintillatorOutputScheme
 */
class RMGDetectorHit : public G4VHit {

  public:

    RMGDetectorHit() = default;
    ~RMGDetectorHit() = default;

    RMGDetectorHit(const RMGDetectorHit&) = default;

    RMGDetectorHit& operator=(RMGDetectorHit const&) = delete;
    RMGDetectorHit(RMGDetectorHit&&) = delete;
    RMGDetectorHit& operator=(RMGDetectorHit&&) = delete;

    bool operator==(const RMGDetectorHit&) const;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    void Print() override;
    void Draw() override;

    /** @brief Remage detector unique identifier (uid) for the volume the hit took place in. */
    int detector_uid = -1;

    /** @brief PDG particle code for the track. */
    int particle_type = -1;

    /** @brief Energy deposited in this step (Geant4 energy units). */
    double energy_deposition = -1;
    /** @brief Distance from the pre-step point to the closest surface of the sensitive volume. */
    double distance_to_surface_prestep = -1;
    /** @brief Distance from the step-midpoint to the closest surface of the sensitive volume. */
    double distance_to_surface_average = -1;
    /** @brief Distance from the post-step point to the closest surface of the sensitive volume. */
    double distance_to_surface_poststep = -1;

    G4ThreeVector global_position_poststep; ///< Step post-point in world coordinates.
    G4ThreeVector global_position_prestep;  ///< Step pre-point in world coordinates.
    G4ThreeVector global_position_average;  ///< Step midpoint in world coordinates.

    double global_time = -1;  ///< Global time at the pre-step point.
    int track_id = -1;        ///< Geant4 track id of the step.
    int parent_track_id = -1; ///< Track id of the parent (0 for primaries).

    G4VPhysicalVolume* physical_volume = nullptr; ///< Physical volume the step took place in.

    double velocity_pre = -1;  ///< Velocity at the pre-step point.
    double velocity_post = -1; ///< Velocity at the post-step point.

    G4Colour fDrawColour = G4Colour(0, 0, 1);
};

using RMGDetectorHitsCollection = G4THitsCollection<RMGDetectorHit>;

/// \cond this triggers a sphinx error
extern G4ThreadLocal G4Allocator<RMGDetectorHit>* RMGDetectorHitAllocator;
/// \endcond

inline void* RMGDetectorHit::operator new(size_t) {
  if (!RMGDetectorHitAllocator) RMGDetectorHitAllocator = new G4Allocator<RMGDetectorHit>;
  return (void*)RMGDetectorHitAllocator->MallocSingle();
}

inline void RMGDetectorHit::operator delete(void* hit) {
  RMGDetectorHitAllocator->FreeSingle(static_cast<RMGDetectorHit*>(hit));
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

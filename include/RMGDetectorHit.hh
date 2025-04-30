// Copyright (C) 2025 Toby Dixon <toby.dixon.23@ucl.ac.uk>
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

    double velocity_pre = -1;
    double velocity_post = -1;

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

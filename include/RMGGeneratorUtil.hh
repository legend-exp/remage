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

#ifndef _RMG_GENERATOR_UTIL_HH
#define _RMG_GENERATOR_UTIL_HH

#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"
#include "globals.hh"

namespace RMGGeneratorUtil {

  /**
   * @brief Check if a given Geant4 solid type is supported for native sampling.
   *
   * This function verifies if the provided solid type (as a string)
   * corresponds to a supported type for random sampling.
   *
   * @param g4_solid_type A string representing the Geant4 solid type.
   * @return true if the solid type is sampleable; false otherwise.
   */
  bool IsSampleable(std::string g4_solid_type);

  /**
   * @brief Generate a random 3D point within or on the surface of the given solid.
   *
   * For a generic @c G4VSolid, this function delegates to the appropriate specialized function
   * based on the solid's type.
   *
   * @param solid Pointer to the Geant4 solid.
   * @param on_surface If true, sample a point on the surface; otherwise, inside the solid.
   * @return A random point as @c G4ThreeVector.
   */
  G4ThreeVector rand(const G4VSolid*, bool on_surface = false);

  /**
   * @brief Generate a random point in or on the surface of a \c G4Box.
   *
   * Calculates a random 3D point either inside the box or on one of its surfaces.
   *
   * @param box Pointer to the \c G4Box.
   * @param on_surface If true, the point lies on the box's surface; if false, it lies within the volume.
   * @return A random point as \c G4ThreeVector.
   */
  G4ThreeVector rand(const G4Box*, bool on_surface = false);

  /**
   * @brief Generate a random point in or on the surface of a \c G4Sphere.
   *
   * Uses spherical coordinate sampling to generate a point either inside the sphere or exactly on its surface.
   *
   * @param sphere Pointer to the \c G4Sphere.
   * @param on_surface If true, sample on the sphere's surface; otherwise, inside it.
   * @return A random point as \c G4ThreeVector.
   */
  G4ThreeVector rand(const G4Sphere*, bool on_surface = false);

  /**
   * @brief Generate a random point in or on the surface of a \c G4Orb.
   *
   * Uses appropriate methods to sample a point within or on the outer surface of the orb.
   *
   * @param orb Pointer to the \c G4Orb.
   * @param on_surface If true, sample a point on the surface; otherwise, inside the volume.
   * @return A random point as \c G4ThreeVector.
   */
  G4ThreeVector rand(const G4Orb*, bool on_surface = false);

  /**
   * @brief Generate a random point in or on the surface of a \c G4Tubs (cylindrical tube).
   *
   * Handles random point generation for cylindrical shapes, accounting for inner and outer radii.
   *
   * @param tub Pointer to the \c G4Tubs.
   * @param on_surface If true, sample on the surface; if false, sample within the volume.
   * @return A random point as \c G4ThreeVector.
   */
  G4ThreeVector rand(const G4Tubs*, bool on_surface = false);
} // namespace RMGGeneratorUtil

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

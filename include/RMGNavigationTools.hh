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

#ifndef _RMG_NAVIGATION_TOOLS_HH_
#define _RMG_NAVIGATION_TOOLS_HH_

#include <regex>
#include <set>
#include <string>
#include <vector>

#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"

// TODO: write function that locates points in global coordinates by using an
// auxiliary G4Navigator. The G4Navigator instance must be unique and its
// access thread-safe.

namespace RMGNavigationTools {

  /**
   * @brief Finds a logical volume by name.
   *
   * Searches the global @c G4LogicalVolumeStore for a logical volume whose name matches the given
   * @c std::string. Returns a pointer to the matching logical volume or @c nullptr if not found.
   *
   * @param name The name of the logical volume to search for.
   * @return Pointer to the logical volume, or @c nullptr if not found.
   */
  G4LogicalVolume* FindLogicalVolume(std::string name);
  /**
   * @brief Finds a physical volume by name and copy number.
   *
   * Searches the global @c G4PhysicalVolumeStore for a physical volume whose name matches the given
   * @c std::string and whose copy number matches the specified value.
   *
   * @param name The name of the physical volume to search for.
   * @param copy_nr The copy number (default is 0).
   * @return Pointer to the physical volume, or @c nullptr if not found.
   */
  G4VPhysicalVolume* FindPhysicalVolume(std::string name, int copy_nr = 0);

  std::set<G4VPhysicalVolume*> FindPhysicalVolumesFromRegex(std::string name, std::string copy_nr = ".*");
  /**
   * @brief Finds the direct mother volume of a given physical volume.
   *
   * Returns the unique physical volume that is the direct mother of the specified volume.
   * If multiple direct mothers are found, a fatal error is logged and the first one is returned.
   *
   * @param volume Pointer to the @c G4VPhysicalVolume whose direct mother is sought.
   * @return Pointer to the direct mother @c G4VPhysicalVolume.
   */
  G4VPhysicalVolume* FindDirectMother(G4VPhysicalVolume* volume);
  /**
   * @brief Finds all direct mother volumes of a given physical volume.
   *
   * Searches the global @c G4PhysicalVolumeStore for all physical volumes whose associated logical
   * volumes are ancestors of the given volume. Then filters out non‐minimal ancestors so that only
   * the direct (minimal) mothers remain.
   *
   * @param volume Pointer to the @c G4VPhysicalVolume whose direct mothers are to be found.
   * @return A @c std::set of pointers to the direct mother @c G4VPhysicalVolume objects.
   */
  std::set<G4VPhysicalVolume*> FindDirectMothers(G4VPhysicalVolume* volume);

  /**
   * @brief Prints all logical volumes in the store.
   *
   * Iterates over the global @c G4LogicalVolumeStore and logs each logical volume along with key
   * properties (number of daughter volumes, material density, mass, volume, pressure, and temperature).
   */
  void PrintListOfLogicalVolumes();
  /**
   * @brief Prints all physical volumes in the store.
   *
   * Iterates over the global @c G4PhysicalVolumeStore, sorts the physical volumes by name and copy
   * number, and logs their names, copy numbers, and the names of their logical parent volumes.
   */
  void PrintListOfPhysicalVolumes();

  struct VolumeTreeEntry {
      VolumeTreeEntry() = delete;
      VolumeTreeEntry(const VolumeTreeEntry&) = default;
      VolumeTreeEntry(G4VPhysicalVolume* pv) { physvol = pv; }

      G4VPhysicalVolume* physvol;

      G4ThreeVector vol_global_translation; // origin
      G4RotationMatrix vol_global_rotation; // identity
      std::vector<G4RotationMatrix> partial_rotations;
      std::vector<G4ThreeVector> partial_translations;
  };

  /**
   * @brief find all ways to reach the world volume from a given physical volume.
   * @param pv the physical volume to start with.
   */
  std::vector<VolumeTreeEntry> FindGlobalPositions(G4VPhysicalVolume* pv);

} // namespace RMGNavigationTools

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

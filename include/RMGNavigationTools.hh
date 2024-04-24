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

#ifndef _RMG_NAVIGATION_TOOLS_HH_
#define _RMG_NAVIGATION_TOOLS_HH_

#include <set>
#include <string>

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

// TODO: write function that locates points in global coordinates by using an
// auxiliary G4Navigator. The G4Navigator instance must be unique and its
// access thread-safe.

namespace RMGNavigationTools {

  G4LogicalVolume* FindLogicalVolume(std::string name);
  G4VPhysicalVolume* FindPhysicalVolume(std::string name, int copy_nr = 0);
  G4VPhysicalVolume* FindDirectMother(G4VPhysicalVolume* volume);
  std::set<G4VPhysicalVolume*> FindDirectMothers(G4VPhysicalVolume* volume);

  void PrintListOfLogicalVolumes();
  void PrintListOfPhysicalVolumes();
} // namespace RMGNavigationTools

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

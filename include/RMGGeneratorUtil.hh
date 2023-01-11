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

  bool IsSampleable(std::string g4_solid_type);

  G4ThreeVector rand(const G4VSolid*, bool on_surface = false);

  G4ThreeVector rand(const G4Box*, bool on_surface = false);

  G4ThreeVector rand(const G4Sphere*, bool on_surface = false);

  G4ThreeVector rand(const G4Orb*, bool on_surface = false);

  G4ThreeVector rand(const G4Tubs*, bool on_surface = false);
} // namespace RMGGeneratorUtil

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

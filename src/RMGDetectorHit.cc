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

#include "RMGDetectorHit.hh"

#include "G4Circle.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"

#include "RMGLog.hh"

/// \cond this triggers a sphinx error
G4ThreadLocal G4Allocator<RMGDetectorHit>* RMGDetectorHitAllocator = nullptr;
/// \endcond

// NOTE: does this make sense?
G4bool RMGDetectorHit::operator==(const RMGDetectorHit& right) const { return this == &right; }

void RMGDetectorHit::Print() {
  RMGLog::Out(RMGLog::debug, "Detector UID: ", this->detector_uid,
      " / Particle: ", this->particle_type,
      " / Energy: ", G4BestUnit(this->energy_deposition, "Energy"),
      " / Position (prestep): ", this->global_position_prestep / CLHEP::m, " m",
      " / Time: ", this->global_time / CLHEP::ns, " ns", " / trackid ", this->track_id);
}

void RMGDetectorHit::Draw() {
  const auto vis_man = G4VVisManager::GetConcreteInstance();
  if (vis_man and this->energy_deposition > 0) {
    G4Circle circle(this->global_position_prestep);
    circle.SetScreenSize(5);
    circle.SetFillStyle(G4Circle::filled);
    circle.SetVisAttributes(G4VisAttributes(fDrawColour));
    vis_man->Draw(circle);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

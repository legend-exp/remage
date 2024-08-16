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

#include "RMGTrackingAction.hh"

#include "G4Track.hh"

#include "RMGRunAction.hh"

RMGTrackingAction::RMGTrackingAction(RMGRunAction* run_action) : fRunAction(run_action) {}

void RMGTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

  for (auto& el : fRunAction->GetAllOutputDataFields()) { el->TrackingActionPre(aTrack); }
}

void RMGTrackingAction::PostUserTrackingAction(const G4Track* /*aTrack*/) {}

// vim: tabstop=2 shiftwidth=2 expandtab

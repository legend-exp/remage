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

#include "RMGStackingAction.hh"

#include "RMGEventAction.hh"

RMGStackingAction::RMGStackingAction(RMGEventAction* eventaction) : fEventAction(eventaction) {}

G4ClassificationOfNewTrack RMGStackingAction::ClassifyNewTrack(const G4Track* /*aTrack*/) {
  return fUrgent;
}

void RMGStackingAction::NewStage() {}

void RMGStackingAction::PrepareNewEvent() {}

// vim: tabstop=2 shiftwidth=2 expandtab

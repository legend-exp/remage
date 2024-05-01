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

#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"

#include "RMGEventAction.hh"
#include "RMGGermaniumOutputScheme.hh"
#include "RMGHardware.hh"
#include "RMGManager.hh"
#include "RMGOpticalOutputScheme.hh"
#include "RMGRunAction.hh"

RMGStackingAction::RMGStackingAction(RMGRunAction* runaction) : fRunAction(runaction) {}

G4ClassificationOfNewTrack RMGStackingAction::ClassifyNewTrack(const G4Track* aTrack) {
  // defer tracking of optical photons.
  if (aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) return fWaiting;
  return fUrgent;
}

void RMGStackingAction::NewStage() {
  auto ge_output = fRunAction->GetOutputDataFields(RMGHardware::DetectorType::kGermanium);
  if (ge_output == nullptr) return;

  bool discard_photons =
      (dynamic_cast<RMGGermaniumOutputScheme&>(*ge_output)).GetDiscardPhotonsIfNoGermaniumEdep();
  if (!discard_photons) return;

  auto run_man = RMGManager::Instance()->GetG4RunManager();
  const auto event = run_man->GetCurrentEvent();
  if (ge_output->ShouldDiscardEvent(event)) {
    // discard all waiting events, as there was no energy deposition in Germanium.
    auto stack_man = G4EventManager::GetEventManager()->GetStackManager();
    stack_man->clear();
  }
}

void RMGStackingAction::PrepareNewEvent() {}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include <optional>

#include "G4EventManager.hh"
#include "G4Track.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGRunAction.hh"

RMGStackingAction::RMGStackingAction(RMGRunAction* runaction) : fRunAction(runaction) {}

G4ClassificationOfNewTrack RMGStackingAction::ClassifyNewTrack(const G4Track* aTrack) {
  std::optional<G4ClassificationOfNewTrack> new_status = std::nullopt;
  for (auto& el : fRunAction->GetAllOutputDataFields()) {
    auto request_status = el->StackingActionClassify(aTrack, fStage);
    if (!request_status.has_value()) continue; // this output scheme does not care.

    if (!new_status.has_value() || new_status.value() == request_status.value()) {
      new_status = request_status;
    } else {
      RMGLog::Out(RMGLog::error,
          "Conflicting requests for new track classification in RMGStackingAction.");
    }
  }

  if (new_status.has_value()) return new_status.value();
  return fUrgent;
}

void RMGStackingAction::NewStage() {
  // we can have only one result from all output schemes, if we have
  std::optional<bool> should_do_stage = std::nullopt;
  for (auto& el : fRunAction->GetAllOutputDataFields()) {
    auto request_stage = el->StackingActionNewStage(fStage);
    if (!request_stage.has_value()) continue; // this output scheme does not care.

    if (!should_do_stage.has_value() || should_do_stage.value() == request_stage.value()) {
      should_do_stage = request_stage;
    } else {
      RMGLog::Out(RMGLog::error,
          "Conflicting requests for new stage termination in RMGStackingAction.");
    }
  }

  if (should_do_stage.has_value() && !should_do_stage.value()) {
    auto stack_man = G4EventManager::GetEventManager()->GetStackManager();
    stack_man->clear();
  }
  fStage++;
}

void RMGStackingAction::PrepareNewEvent() { fStage = 0; }

// vim: tabstop=2 shiftwidth=2 expandtab

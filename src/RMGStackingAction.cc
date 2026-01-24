// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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
  std::chrono::high_resolution_clock::time_point start_time;
  if (RMGLog::GetLogLevel() <= RMGLog::debug) {
    start_time = std::chrono::high_resolution_clock::now();
  }

  std::optional<G4ClassificationOfNewTrack> new_status = std::nullopt;
  
  RMGLog::Out(RMGLog::debug_event, "Looping over output schemes for track ", aTrack->GetTrackID(), " of particle type ", aTrack->GetDefinition()->GetParticleName());
  for (auto& el : fRunAction->GetAllOutputDataFields()) {
    RMGLog::Out(RMGLog::debug_event, "Asking output scheme ", typeid(*el).name());
    auto request_status = el->StackingActionClassify(aTrack, fStage);
    if (!request_status.has_value()) continue; // this output scheme does not care.

    if (!new_status.has_value() || new_status.value() == request_status.value()) {
      new_status = request_status;
    } else {
      RMGLog::OutDev(RMGLog::error, "Conflicting requests for new track classification.");
    }
  }

  if (RMGLog::GetLogLevel() <= RMGLog::debug) {
    auto end_time = std::chrono::high_resolution_clock::now();
    fClassifyTotalTime += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    fClassifyCallCount++;
  }

  if (new_status.has_value()) return new_status.value();
  return fUrgent;
}

void RMGStackingAction::NewStage() {
  // we can have only one result from all output schemes; if we have conflicting requests, we cannot continue.
  std::optional<bool> should_do_stage = std::nullopt;
  for (auto& el : fRunAction->GetAllOutputDataFields()) {
    auto request_stage = el->StackingActionNewStage(fStage);
    if (!request_stage.has_value()) continue; // this output scheme does not care.

    if (!should_do_stage.has_value() || should_do_stage.value() == request_stage.value()) {
      should_do_stage = request_stage;
    } else {
      RMGLog::OutDev(RMGLog::error, "Conflicting requests for new stage termination.");
    }
  }

  auto stack_man = G4EventManager::GetEventManager()->GetStackManager();
  RMGLog::Out(
    RMGLog::debug,
    "Tracks moved from Waiting to Urgent stack. Size: ", stack_man->GetNUrgentTrack()
  );

  if (should_do_stage.has_value() && !should_do_stage.value()) {
    //RMGLog::Out(
    //    RMGLog::debug,
    //    "Freeing up urgent stack of size ", stack_man->GetNUrgentTrack(), " and waiting stack of size ", stack_man->GetNWaitingTrack()
    //  );
    stack_man->clear();
  }
  fStage++;
}

void RMGStackingAction::PrepareNewEvent() {
  // Output timing statistics from previous event
  if (RMGLog::GetLogLevel() <= RMGLog::debug && fClassifyCallCount > 0) {
    auto avg_time_ns = fClassifyTotalTime.count() / fClassifyCallCount;
    RMGLog::Out(
      RMGLog::debug,
      "ClassifyNewTrack called ", fClassifyCallCount, " times. "
      "Average time: ", avg_time_ns, " ns (",
      avg_time_ns / 1000.0, " μs)"
    );
  }

  // Reset for new event
  fStage = 0;
  if (RMGLog::GetLogLevel() <= RMGLog::debug) {
    fClassifyCallCount = 0;
    fClassifyTotalTime = std::chrono::nanoseconds{0};
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

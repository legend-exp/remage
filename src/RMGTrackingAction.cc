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

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4RadioactiveDecay.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"

#include "RMGLog.hh"
#include "RMGRunAction.hh"

RMGTrackingAction::RMGTrackingAction(RMGRunAction* run_action) : fRunAction(run_action) {

  this->DefineCommands();
}

void RMGTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

  for (auto& el : fRunAction->GetAllOutputDataFields()) { el->TrackingActionPre(aTrack); }
}

void RMGTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {

  if (fResetInitialDecayTime) ResetInitialDecayTime(aTrack);
}

void RMGTrackingAction::ResetInitialDecayTime(const G4Track* aTrack) {

  // only nuclei in the first step are eligible to be reset.
  if (aTrack->GetTrackID() != 1 || aTrack->GetParentID() != 0) return;
  if (aTrack->GetDefinition()->GetParticleType() != "nucleus") return;

  // only reset the time if the last process is a radioactive decay.
  auto creator_process = aTrack->GetStep()->GetPostStepPoint()->GetProcessDefinedStep();
  if (!dynamic_cast<const G4RadioactiveDecay*>(creator_process)) return;

  const auto secondaries = fpTrackingManager->GimmeSecondaries();
  auto secondaries_in_current_step = aTrack->GetStep()->GetNumberOfSecondariesInCurrentStep();
  // if we have more secondaries than from the final decay, the earlier ones will have larger times.
  if (secondaries_in_current_step != secondaries->size()) {
    RMGLog::Out(RMGLog::warning, "inconsistent (non-monotonous) timing in event ",
        G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID());
  }

  for (auto sec : *secondaries) { sec->SetGlobalTime(0.); }
}

void RMGTrackingAction::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Processes/Stepping/",
      "Commands for controlling physics processes");

  fMessenger->DeclareProperty("ResetInitialDecayTime", fResetInitialDecayTime)
      .SetGuidance("If the initial step is a radioactive decay, reset the global time of all its "
                   "secondary tracks to 0.")
      .SetDefaultValue("false")
      .SetStates(G4State_PreInit);
}

// vim: tabstop=2 shiftwidth=2 expandtab

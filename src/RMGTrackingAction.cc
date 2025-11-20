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

#include "RMGTrackingAction.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4RadioactiveDecay.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"

#include "RMGLog.hh"
#include "RMGRunAction.hh"

RMGTrackingAction::RMGTrackingAction(RMGRunAction* run_action) : fRunAction(run_action) {

  this->SetLongGlobalTimeUncertaintyWarning(1 * CLHEP::us);
  this->DefineCommands();
}

void RMGTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

  for (auto& el : fRunAction->GetAllOutputDataFields()) { el->TrackingActionPre(aTrack); }
}

void RMGTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {

  for (auto& el : fRunAction->GetAllOutputDataFields()) { el->TrackingActionPost(aTrack); }

  bool check_global_time = true;
  if (fResetInitialDecayTime) { check_global_time = !ResetInitialDecayTime(aTrack); }

  if (check_global_time && !fHadLongTimeWarning &&
      aTrack->GetGlobalTime() > fMaxRepresentableGlobalTime) {
    RMGLog::Out(
        RMGLog::warning,
        "encountered long global time (> ",
        fMaxRepresentableGlobalTime / CLHEP::year,
        " yr). Global time precision might be worse than 1 us."
    );
    fHadLongTimeWarning = true;
  }
}

bool RMGTrackingAction::ResetInitialDecayTime(const G4Track* aTrack) {

  // only nuclei in the first step are eligible to be reset.
  if (aTrack->GetTrackID() != 1 || aTrack->GetParentID() != 0) return false;
  if (aTrack->GetDefinition()->GetParticleType() != "nucleus") return false;

  // only reset the time if the last process is a radioactive decay.
  auto creator_process = aTrack->GetStep()->GetPostStepPoint()->GetProcessDefinedStep();
  if (!dynamic_cast<const G4RadioactiveDecay*>(creator_process)) return false;

  const auto secondaries = fpTrackingManager->GimmeSecondaries();
  auto secondaries_in_current_step = aTrack->GetStep()->GetNumberOfSecondariesInCurrentStep();
  // if we have more secondaries than from the final decay, the earlier ones will have larger times.
  if (secondaries_in_current_step != secondaries->size()) {
    RMGLog::Out(
        RMGLog::warning,
        "inconsistent (non-monotonous) timing in event ",
        G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
    );
  }

  for (auto sec : *secondaries) { sec->SetGlobalTime(0.); }

  return true;
}

void RMGTrackingAction::SetLongGlobalTimeUncertaintyWarning(double uncert) {

  // this is just a good "guess" that might not hold true in all cases, i.e. some us values
  // might still not be unique below this.
  fMaxRepresentableGlobalTime = std::pow(2, std::numeric_limits<double>::digits) * uncert;
}

void RMGTrackingAction::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Processes/Stepping/",
      "Commands for controlling physics processes"
  );

  fMessenger->DeclareProperty("ResetInitialDecayTime", fResetInitialDecayTime)
      .SetGuidance(
          "If the initial step is a radioactive decay, reset the global time of all its "
          "secondary tracks to 0."
      )
      .SetGuidance(
          std::string("This is ") + (fResetInitialDecayTime ? "enabled" : "disabled") + " by default"
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit(
          "LargeGlobalTimeUncertaintyWarning",
          "us",
          &RMGTrackingAction::SetLongGlobalTimeUncertaintyWarning
      )
      .SetGuidance(
          "Warn if the global times of tracks get too large to provide the requested time "
          "uncertainty."
      )
      .SetGuidance("Uses 1 us by default")
      .SetDefaultValue("1")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

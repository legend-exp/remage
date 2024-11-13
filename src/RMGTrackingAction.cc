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

namespace {
  template<typename T> constexpr double const_pow(T base, T exp) {
    return exp == 0 ? 1 : base * const_pow(base, exp - 1);
  }
} // namespace

RMGTrackingAction::RMGTrackingAction(RMGRunAction* run_action) : fRunAction(run_action) {

  this->DefineCommands();
}

void RMGTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

  for (auto& el : fRunAction->GetAllOutputDataFields()) { el->TrackingActionPre(aTrack); }
}

void RMGTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {

  bool check_global_time = true;
  if (fResetInitialDecayTime) { check_global_time = !ResetInitialDecayTime(aTrack); }

  // this is just a good "guess" that might not hold true in all cases, i.e. some us values
  // might still not be unique below this.
  constexpr double max_representable_time_with_us_prec =
      const_pow(2, std::numeric_limits<double>::digits) * CLHEP::us;

  if (check_global_time && !fHadLongTimeWarning) {
    if (aTrack->GetGlobalTime() > max_representable_time_with_us_prec) {
      RMGLog::Out(RMGLog::warning, "encountered long global time (> ",
          max_representable_time_with_us_prec / CLHEP::year,
          " yr). Global time precision might be worse than 1 us.");
      fHadLongTimeWarning = true;
    }
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
    RMGLog::Out(RMGLog::warning, "inconsistent (non-monotonous) timing in event ",
        G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID());
  }

  for (auto sec : *secondaries) { sec->SetGlobalTime(0.); }

  return true;
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

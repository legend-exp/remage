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

#include "RMGIsotopeFilterScheme.hh"

#include <set>

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"

#include "RMGLog.hh"

RMGIsotopeFilterScheme::RMGIsotopeFilterScheme() { this->DefineCommands(); }

void RMGIsotopeFilterScheme::TrackingActionPre(const G4Track* aTrack) {
  const auto particle = aTrack->GetParticleDefinition();
  if (!particle->IsGeneralIon()) return;
  const int z = particle->GetAtomicNumber();
  const int a = particle->GetAtomicMass();
  if (z == 0 || a == 0) return; // not an isotope, but any other particle.

  if (fIsotopes.find({a, z}) == fIsotopes.end()) return;

  auto event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
  auto info = event->GetUserInformation();
  if (info != nullptr && dynamic_cast<RMGIsotopeFilterEventInformation*>(info) == nullptr) {
    RMGLog::Out(
        RMGLog::error,
        "other user info class found, instead of RMGIsotopeFilterEventInformation"
    );
    return;
  }
  if (info == nullptr) event->SetUserInformation(new RMGIsotopeFilterEventInformation());
}

// invoked in RMGEventAction::EndOfEventAction()
bool RMGIsotopeFilterScheme::ShouldDiscardEvent(const G4Event* event) {
  // exit fast if no threshold is configured.
  if (fIsotopes.empty()) return false;

  auto info = event->GetUserInformation();
  if (info != nullptr && dynamic_cast<RMGIsotopeFilterEventInformation*>(info) == nullptr) {
    // Someone else tries to set Event user information.
    RMGLog::Out(
        RMGLog::error,
        "other user info class found, instead of RMGIsotopeFilterEventInformation"
    );
    return false;
  }
  return info == nullptr;
}

std::optional<G4ClassificationOfNewTrack> RMGIsotopeFilterScheme::StackingActionClassify(
    const G4Track* aTrack,
    int stage
) {
  // we are only interested in stacking optical photons into stage 1 after stage 0 finished.
  if (stage != 0) return std::nullopt;

  // defer tracking of optical photons.
  if (fDiscardPhotonsIfIsotopeNotProduced &&
      aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition())
    return fWaiting;
  return std::nullopt;
}

std::optional<bool> RMGIsotopeFilterScheme::StackingActionNewStage(const int stage) {
  // we are only interested in stacking optical photons into stage 1 after stage 0 finished.
  if (stage != 0) return std::nullopt;
  // if we do not want to discard any photons ourselves, let other output schemes decide (i.e. not
  // force `true` on them).
  if (!fDiscardPhotonsIfIsotopeNotProduced) return std::nullopt;

  const auto event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
  // discard all waiting events, if there were none of the requested isotopes produced.
  return ShouldDiscardEvent(event) ? std::make_optional(false) : std::nullopt;
}

void RMGIsotopeFilterScheme::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Output/IsotopeFilter/",
      "Commands for filtering event out by created isotopes."
  );

  fMessenger->DeclareMethod("AddIsotope", &RMGIsotopeFilterScheme::AddIsotope)
      .SetGuidance(
          "Add an isotope to the list. Only events that have a track with this isotope at "
          "any point in time will be persisted."
      )
      .SetParameterName(0, "A", false, false)
      .SetParameterName(1, "Z", false, false)
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareProperty("DiscardPhotonsIfIsotopeNotProduced", fDiscardPhotonsIfIsotopeNotProduced)
      .SetGuidance(
          "Discard optical photons (before simulating them), if the specified isotopes "
          "had not been produced in the same event."
      )
      .SetGuidance(
          "note: If another output scheme also requests the photons to be discarded, the "
          "isotope filter does not force the photons to be simulated."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

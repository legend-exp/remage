// Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#include "RMGStagingScheme.hh"

#include "G4Electron.hh"
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"

#include "RMGOutputTools.hh"

RMGStagingScheme::RMGStagingScheme() { this->DefineCommands(); }

std::optional<G4ClassificationOfNewTrack> RMGStagingScheme::StackingActionClassify(
    const G4Track* aTrack,
    int stage
) {
  if (stage != 0) return std::nullopt; // only apply staging logic in stage 0

  if (aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    return Classify_OpticalPhoton(aTrack);
  } else if (aTrack->GetDefinition() == G4Electron::Definition()) {
    return Classify_Electron(aTrack);
  }
  return std::nullopt;
}

void RMGStagingScheme::SteppingAction(const G4Step* step) {

  auto track = step->GetTrack();
  if (track == nullptr) return;

  // Keep primaries unaffected, matching existing staging behavior.
  if (track->GetParentID() == 0) return;

  if (track->GetTrackStatus() != fAlive) return;

  if (track->GetDefinition() != G4Electron::Definition()) return;

  const bool suspend_on_drop = fSuspendElectronsOnEnergyDrop;
  const double threshold = fElectronMaxEnergyThresholdForStacking;

  if (!suspend_on_drop || threshold < 0) return;

  const auto* pre_step = step->GetPreStepPoint();
  const auto* post_step = step->GetPostStepPoint();
  if (pre_step == nullptr || post_step == nullptr) return;

  const auto pre_energy = pre_step->GetKineticEnergy();
  const auto post_energy = post_step->GetKineticEnergy();

  // Suspend exactly at threshold crossing to avoid repeatedly suspending tracks that always stay
  // below threshold.
  if (pre_energy > threshold && post_energy <= threshold) track->SetTrackStatus(fSuspend);
}

std::optional<G4ClassificationOfNewTrack> RMGStagingScheme::Classify_OpticalPhoton(
    const G4Track* aTrack
) const {
  // Electron staging also implies optical-photon staging (see DeferToWaitingStage guidance).
  if ((fDeferOpticalPhotonsToWaitingStage) &&
      aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    return fWaiting;
  }
  return std::nullopt;
}

std::optional<G4ClassificationOfNewTrack> RMGStagingScheme::Classify_Electron(
    const G4Track* aTrack
) const {
  if (!fDeferElectronsToWaitingStage) return std::nullopt;

  // do not touch the primary track of an event.
  if (aTrack->GetParentID() == 0) return std::nullopt;

  // if a max energy threshold is set, only defer tracks below that threshold.
  if (fElectronMaxEnergyThresholdForStacking >= 0 &&
      aTrack->GetKineticEnergy() > fElectronMaxEnergyThresholdForStacking)
    return std::nullopt;

  // if a min energy threshold is set, only defer tracks above that threshold.
  if (fElectronMinEnergyThresholdForStacking >= 0 &&
      aTrack->GetKineticEnergy() < fElectronMinEnergyThresholdForStacking)
    return std::nullopt;

  const auto* volume = aTrack->GetVolume();
  if (volume == nullptr) return std::nullopt;

  // If volume names are configured, only apply electron staging inside those volumes.
  // (If none are configured, electron staging applies to all volumes.)
  if (!fElectronVolumeNames.empty()) {
    const auto vol_name = volume->GetLogicalVolume()->GetName();
    if (!fElectronVolumeNames.contains(vol_name)) return std::nullopt;
  }

  // stop if electron safety is not configured.
  if (fElectronVolumeSafety < 0) return std::nullopt;

  // if safety is zero, always defer.
  if (fElectronVolumeSafety == 0) return fWaiting;

  // only defer tracks that have a minimum distance to Germanium detector surfaces.
  bool is_within_safety = RMGOutputTools::is_within_surface_safety(
      volume,
      aTrack->GetPosition(),
      fElectronVolumeSafety,
      /*is_distance_check_germanium_only=*/true
  );
  if (is_within_safety) return std::nullopt;

  return fWaiting;
}

void RMGStagingScheme::DefineCommands() {

  fOpticalPhotonStagingMessengers = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Staging/OpticalPhotons/",
      "Commands for staging optical photon tracks."
  );

  fOpticalPhotonStagingMessengers
      ->DeclareProperty("DeferToWaitingStage", fDeferOpticalPhotonsToWaitingStage)
      .SetGuidance("Defer optical photons to the waiting stack during stage 0.")
      .SetGuidance(
          std::string("This is ") + (fDeferOpticalPhotonsToWaitingStage ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Staging/Electrons/",
      "Commands for staging electron tracks."
  );

  fElectronStagingMessengers->DeclareProperty("DeferToWaitingStage", fDeferElectronsToWaitingStage)
      .SetGuidance("Defer secondary electrons to the waiting stack during stage 0.")
      .SetGuidance(
          std::string("This is ") + (fDeferElectronsToWaitingStage ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethodWithUnit("VolumeSafety", "cm", &RMGStagingScheme::SetElectronVolumeSafety)
      .SetGuidance(
          "Set the minimum distance to a Germanium detector surface for this electron to be staged."
      )
      .SetGuidance("Set to 0 (the default) to stage regardless of surface distance.")
      .SetParameterName("safety", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethod("AddVolumeName", &RMGStagingScheme::AddElectronVolumeName)
      .SetGuidance("Add a volume name in which electron staging is active.")
      .SetGuidance("If this command is not called, electron staging applies to all volumes.")
      .SetParameterName("volume", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethodWithUnit(
          "MaxEnergyThresholdForStacking",
          "MeV",
          &RMGStagingScheme::SetElectronMaxEnergyThresholdForStacking
      )
      .SetGuidance("Set the maximum kinetic energy for e- tracks to be considered for staging.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethodWithUnit(
          "MinEnergyThresholdForStacking",
          "MeV",
          &RMGStagingScheme::SetElectronMinEnergyThresholdForStacking
      )
      .SetGuidance("Set the minimum kinetic energy for e- tracks to be considered for staging.")
      .SetGuidance("Useful to skip staging low-energy electrons (e.g. below the Cherenkov threshold).")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers->DeclareProperty("SuspendOnEnergyDrop", fSuspendElectronsOnEnergyDrop)
      .SetGuidance("Suspend secondary electrons when they cross from above to below the configured kinetic-energy threshold.")
      .SetGuidance("The threshold is taken from MaxEnergyThresholdForStacking.")
      .SetGuidance(
          std::string("This is ") + (fSuspendElectronsOnEnergyDrop ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("false")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include "RMGOutputTools.hh"

RMGStagingScheme::RMGStagingScheme() { this->DefineCommands(); }

std::optional<G4ClassificationOfNewTrack> RMGStagingScheme::StackingActionClassify(
    const G4Track* aTrack,
    int stage
) {
  // we are only interested in staging into waiting at stage 0.
  if (stage != 0) return std::nullopt;

  if (fDeferOpticalPhotonsToWaitingStage &&
      aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    return fWaiting;
  }

  if (!fDeferElectronsToWaitingStage) return std::nullopt;

  // do not touch the primary track of an event.
  if (aTrack->GetParentID() == 0) return std::nullopt;

  // only defer electron tracks.
  if (aTrack->GetDefinition() != G4Electron::Definition()) return std::nullopt;

  // if a max energy threshold is set, only defer tracks below that threshold.
  if (fElectronMaxEnergyThresholdForStacking >= 0 &&
      aTrack->GetKineticEnergy() > fElectronMaxEnergyThresholdForStacking)
    return std::nullopt;

  const auto* volume = aTrack->GetVolume();
  if (volume == nullptr) return std::nullopt;

  // If no volume names are configured, apply electron staging to all volumes.
  if (!fElectronVolumeNames.empty()) {
    const auto vol_name = volume->GetLogicalVolume()->GetName();
    if (fElectronVolumeNames.count(vol_name) == 0) return std::nullopt;
  }

  // stop if electron safety is not configured.
  if (fElectronVolumeSafety < 0) return std::nullopt;

  // if safety is zero, always defer.
  if (fElectronVolumeSafety == 0) return fWaiting;

  // only defer tracks that have a minimum distance to other volumes.
  bool is_within_safety = RMGOutputTools::is_within_surface_safety(
      volume,
      aTrack->GetPosition(),
      fElectronVolumeSafety
  );
  if (is_within_safety) return std::nullopt;


  return fWaiting;
}

void RMGStagingScheme::SetDistanceCheckGermaniumOnly(bool enable) {
  RMGOutputTools::SetDistanceCheckGermaniumOnly(enable);
}

void RMGStagingScheme::DefineCommands() {

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(
          this,
          "/RMG/Staging/OpticalPhotons/",
          "Commands for staging optical photon tracks."
      )
  );

  fMessengers.back()
      ->DeclareProperty("DeferToWaitingStage", fDeferOpticalPhotonsToWaitingStage)
      .SetGuidance("Defer optical photons to the waiting stack during stage 0.")
      .SetGuidance(
          std::string("This is ") + (fDeferOpticalPhotonsToWaitingStage ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("false")
      .SetStates(G4State_Idle);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(
          this,
          "/RMG/Staging/Electrons/",
          "Commands for staging electron tracks."
      )
  );

  fMessengers.back()
      ->DeclareProperty("DeferToWaitingStage", fDeferElectronsToWaitingStage)
      .SetGuidance("Defer secondary electrons to the waiting stack during stage 0.")
      .SetGuidance(
          std::string("This is ") + (fDeferElectronsToWaitingStage ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("false")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("VolumeSafety", "cm", &RMGStagingScheme::SetElectronVolumeSafety)
      .SetGuidance("Set the minimum distance to any other volume for this electron to be staged.")
      .SetGuidance("Set to 0 to stage regardless of surface distance.")
      .SetParameterName("safety", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethod("AddVolumeName", &RMGStagingScheme::AddElectronVolumeName)
      .SetGuidance("Add a volume name in which electron staging is active.")
      .SetGuidance("If this command is not called, electron staging applies to all volumes.")
      .SetParameterName("volume", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethod("DistanceCheckGermaniumOnly", &RMGStagingScheme::SetDistanceCheckGermaniumOnly)
      .SetGuidance("Enable/disable Germanium-only filtering for electron surface distance checks.")
      .SetGuidance("When true, only daughter volumes registered as Germanium detectors are considered.")
      .SetGuidance(
          std::string("This is ") +
          (RMGOutputTools::GetDistanceCheckGermaniumOnly() ? "enabled" : "disabled") + " by default."
      )
      .SetParameterName("enable", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit(
          "MaxEnergyThresholdForStacking",
          "MeV",
          &RMGStagingScheme::SetElectronMaxEnergyThresholdForStacking
      )
      .SetGuidance("Set the maximum kinetic energy for e- tracks to be considered for staging.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

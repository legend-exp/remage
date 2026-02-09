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

#include "RMGVolumeDistanceStacker.hh"

#include "G4Electron.hh"
#include "G4Positron.hh"

#include "RMGHardware.hh"
#include "RMGManager.hh"
#include "RMGOutputTools.hh"

namespace u = CLHEP;

RMGVolumeDistanceStacker::RMGVolumeDistanceStacker() { this->DefineCommands(); }

std::optional<G4ClassificationOfNewTrack> RMGVolumeDistanceStacker::StackingActionClassify(
    const G4Track* aTrack,
    int stage
) {
  // we are only interested in stacking tracks into stage 1 after stage 0 finished.
  if (stage != 0) return std::nullopt;

  // do not touch the initial track of an event.
  if (aTrack->GetTrackID() == 0) return std::nullopt;

  // stop if not configured.
  if (fVolumeName.empty() || fVolumeSafety < 0) return std::nullopt;

  // only defer electron/positron tracks.
  if (aTrack->GetDefinition() != G4Electron::Definition() &&
      aTrack->GetDefinition() != G4Positron::Definition())
    return std::nullopt;

  // note: aTrack->GetLogicalVolumeAtVertex() and aTrack->GetVertexPosition() might not be correctly
  // set at this point - do not trust it.

  // only defer tracks in the specified volume.
  const auto vol_name = aTrack->GetVolume()->GetLogicalVolume()->GetName();
  if (vol_name != fVolumeName) return std::nullopt;

  // only defer tracks that have a minimum distance to other volumes.
  auto distance = RMGOutputTools::distance_to_surface(aTrack->GetVolume(), aTrack->GetPosition());
  if (distance < fVolumeSafety) return std::nullopt;

  return fWaiting;
}


void RMGVolumeDistanceStacker::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Output/VolumeStacker/",
      "Commands for controlling stacking tracks in the bulk of a volume."
  );

  fMessenger->DeclareMethodWithUnit("VolumeSafety", "cm", &RMGVolumeDistanceStacker::SetVolumeSafety)
      .SetGuidance("Set the minimum distance to any other volume for this track to be stacked.")
      .SetParameterName("safety", false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("VolumeName", &RMGVolumeDistanceStacker::SetVolumeName)
      .SetGuidance("Set the volume name in which to stack e-/e+ tracks.")
      .SetParameterName("volume", false)
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

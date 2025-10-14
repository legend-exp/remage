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

#include "RMGGeometryCheckOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Geantino.hh"
#include "G4Step.hh"
#include "G4Track.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOutputManager.hh"

namespace u = CLHEP;

void RMGGeometryCheckOutputScheme::SteppingAction(const G4Step* step) {

  const auto prestep = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
  const auto poststep = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume();
  auto info = dynamic_cast<GeantinoUserTrackInformation*>(step->GetTrack()->GetUserInformation());
  if (info->fIsOutside) return;

  const auto last = info->fVolumeStack.back();
  if (last != prestep) {
    RMGLog::Out(
        RMGLog::error,
        "last post-step",
        VolName(last),
        " != pre-step ",
        VolName(prestep),
        ", steps: ",
        VolString(info->fSteps)
    );
  }
  const auto hw = RMGManager::Instance()->GetDetectorConstruction();
  if (last != poststep) {
    if (poststep == nullptr && prestep == hw->GetDefinedWorldVolume()) {
      // exit the world from inside the world volume.
      info->fVolumeStack.pop_back();
    } else if (info->fVolumeStack.size() >= 2 &&
               info->fVolumeStack[info->fVolumeStack.size() - 2] == poststep) {
      // exit again into a previous volume.
      info->fVolumeStack.pop_back();
    } else {
      // check if the new volume is a daughter.
      if (!last->GetLogicalVolume()->IsDaughter(poststep)) {

        // we can still leave this volume for the parent, and will not track this particle further.
        if (last == info->fVolumeStack[0] && poststep->GetLogicalVolume()->IsDaughter(last)) {
          info->fVolumeStack.pop_back();
          info->fIsOutside = true;
          return;
        }

        RMGLog::Out(
            RMGLog::error,
            "post-step ",
            VolName(poststep),
            " is no daughter of pre-step ",
            VolName(last),
            ", steps: ",
            VolString(info->fSteps)
        );
      }
      info->fVolumeStack.push_back(poststep);
    }
  }
  info->fSteps.push_back(poststep);
}

void RMGGeometryCheckOutputScheme::TrackingActionPre(const G4Track* aTrack) {

  auto info = new GeantinoUserTrackInformation();
  aTrack->SetUserInformation(info);
  info->fVolumeStack.push_back(aTrack->GetVolume());

  if (aTrack->GetDefinition() != G4Geantino::GeantinoDefinition()) {
    RMGLog::Out(RMGLog::warning, "did not use geantino as primary, geometry check will not work.");
  }
}

void RMGGeometryCheckOutputScheme::TrackingActionPost(const G4Track* aTrack) {

  auto info = dynamic_cast<GeantinoUserTrackInformation*>(aTrack->GetUserInformation());
  if (!info->fVolumeStack.empty() && info->fVolumeStack.back() != nullptr) {
    RMGLog::Out(
        RMGLog::error,
        "volume stack not empty (count: ",
        info->fVolumeStack.size(),
        ", ",
        VolString(info->fVolumeStack),
        "), steps: ",
        VolString(info->fSteps)
    );
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

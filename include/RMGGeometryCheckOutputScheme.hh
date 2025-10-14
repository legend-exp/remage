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

#ifndef _RMG_GEOMETRY_CHECK_OUTPUT_SCHEME_HH_
#define _RMG_GEOMETRY_CHECK_OUTPUT_SCHEME_HH_

#include <vector>

#include "G4VPhysicalVolume.hh"
#include "G4VUserTrackInformation.hh"

#include "RMGVOutputScheme.hh"

class G4Event;
/**
 * @brief "Output scheme" for checking the geometry with geantinos.
 *
 * For this to work, geantinos must be created (with isotropic emission) in the world volume, but
 * outside the main geometry. They have to have an energy larger than the limits in the production cuts.
 *
 * This is based on an approach described by Jason Detwiler.
 */
class RMGGeometryCheckOutputScheme : public RMGVOutputScheme {

  public:

    RMGGeometryCheckOutputScheme() = default;

    void SteppingAction(const G4Step*) override;
    void TrackingActionPre(const G4Track* aTrack) override;
    void TrackingActionPost(const G4Track* aTrack) override;

  private:

    [[nodiscard]] std::string VolString(const std::vector<G4VPhysicalVolume*> vols) const {
      std::string text;
      for (const auto& s : vols) { text += VolName(s) + ' '; }
      return text;
    };
    [[nodiscard]] std::string VolName(G4VPhysicalVolume* vol) const {
      return vol ? vol->GetName() : "(null)";
    }

    class GeantinoUserTrackInformation : public G4VUserTrackInformation {

      public:

        std::vector<G4VPhysicalVolume*> fVolumeStack;
        std::vector<G4VPhysicalVolume*> fSteps;
        bool fIsOutside = false;
    };
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

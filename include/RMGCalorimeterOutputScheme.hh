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

#ifndef _RMG_CALORIMETER_OUTPUT_SCHEME_HH_
#define _RMG_CALORIMETER_OUTPUT_SCHEME_HH_

#include <memory>
#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGCalorimeterDetector.hh"
#include "RMGDetectorHit.hh"
#include "RMGOutputTools.hh"
#include "RMGVOutputScheme.hh"

class G4Event;
/** @brief Output scheme for Calorimeters.
 *
 *  @details This output scheme records the hits in the Calorimeters.
 *  The properties of each @c RMGDetectorHit are recorded:
 *  - event index,
 *  - time,
 *  - energy deposition,
 */
class RMGCalorimeterOutputScheme : public RMGVOutputScheme {

  public:

    RMGCalorimeterOutputScheme() {};

    /** @brief Sets the names of the output columns, invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager* ana_man) override;

    /** @brief Store the information from the event, invoked in @c RMGEventAction::EndOfEventAction
     * @details Only steps with non-zero energy are stored, unless @c fDiscardZeroEnergyHits is false.
     */
    void StoreEvent(const G4Event* event) override;

  protected:

    [[nodiscard]] std::string GetNtupleNameFlat() const override { return "calorimeter"; }

  private:

    RMGDetectorHitsCollection* GetHitColl(const G4Event*);
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

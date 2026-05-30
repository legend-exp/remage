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

#ifndef _RMG_OPTICAL_OUTPUT_SCHEME_HH_
#define _RMG_OPTICAL_OUTPUT_SCHEME_HH_

#include <vector>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

class G4Event;
/**
 * @brief Output scheme writing optical photon detector hits.
 */
class RMGOpticalOutputScheme : public RMGVOutputScheme {

  public:

    RMGOpticalOutputScheme();

    /** @brief Register the columns of the @c optical ntuple with the analysis manager. */
    void AssignOutputNames(G4AnalysisManager*) override;
    /** @brief Fill one row with the optical photon counts. */
    void StoreEvent(const G4Event*) override;

  protected:

    [[nodiscard]] std::string GetNtupleNameFlat() const override { return "optical"; }

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    bool fStoreSinglePrecisionEnergy = true;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

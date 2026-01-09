// Copyright (C) 2025 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
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

#ifndef _RMG_GEOMBENCH_OUTPUT_SCHEME_HH_
#define _RMG_GEOMBENCH_OUTPUT_SCHEME_HH_

#include <memory>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

class G4Event;

/** @brief Output scheme for geometry navigation benchmark data.
 *
 *  @details This output scheme records timing information for geometry navigation
 *  benchmarking across three orthogonal planes (XZ, YZ, XY). For each pixel in the
 *  benchmark grid, it stores:
 *  - Position coordinates (X, Y, Z depending on the plane)
 *  - Navigation time in seconds
 *
 *  The benchmark data is stored in three separate auxiliary ntuples, one for each plane.
 */
class RMGGeomBenchOutputScheme : public RMGVOutputScheme {

  public:

    RMGGeomBenchOutputScheme();

    /** @brief Sets the names of the output columns, invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager* ana_man) override;

    /** @brief Store benchmark pixel data - called when a pixel completes sampling */
    void SavePixel(int plane_id, double x, double y, double z, double time);

    /** @brief Always store benchmark data regardless of event filtering */
    [[nodiscard]] bool StoreAlways() const override { return true; }

  protected:

    [[nodiscard]] std::string GetNtupleName(RMGDetectorMetadata) const override {
      throw std::logic_error("benchmark output scheme has no detectors");
    }

  private:

    // Ntuple IDs for the three benchmark planes (XZ, YZ, XY)
    int fNtupleIDs[3] = {-1, -1, -1};
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

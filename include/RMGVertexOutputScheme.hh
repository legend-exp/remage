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

#ifndef _RMG_VERTEX_OUTPUT_SCHEME_HH_
#define _RMG_VERTEX_OUTPUT_SCHEME_HH_

#include <vector>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

class G4Event;
class RMGVertexOutputScheme : public RMGVOutputScheme {

  public:

    RMGVertexOutputScheme();

    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;

    // always store vertex data, so that results are not skewed if events are discarded.
    [[nodiscard]] inline bool StoreAlways() const override { return true; }

    inline std::string GetNtupleName(int) const override {
      throw std::logic_error("vertex output scheme has no detectors");
    }

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    bool fStorePrimaryParticleInformation = false;
    bool fSkipPrimaryVertexOutput = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

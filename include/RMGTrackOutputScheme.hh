// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#ifndef _RMG_TRACK_OUTPUT_SCHEME_HH_
#define _RMG_TRACK_OUTPUT_SCHEME_HH_

#include <map>
#include <set>
#include <vector>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

class G4Event;
class G4Track;
class RMGTrackOutputScheme : public RMGVOutputScheme {

  public:

    RMGTrackOutputScheme();

    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void TrackingActionPre(const G4Track*) override;
    void EndOfRunAction(const G4Run*) override;

    // always store vertex data, so that results are not skewed if events are discarded.
    [[nodiscard]] bool StoreAlways() const override { return true; }

  protected:

    [[nodiscard]] std::string GetNtupleName(RMGDetectorMetadata) const override {
      throw std::logic_error("step output scheme has no detectors");
    }

    void AddParticleFilter(const int pdg) { fFilterParticle.insert(pdg); }
    void AddProcessFilter(const std::string proc) { fFilterProcess.insert(proc); }
    void SetEnergyFilter(double energy) { fFilterEnergy = energy; }

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    bool fStoreSinglePrecisionEnergy = false;
    bool fStoreSinglePrecisionPosition = false;

    std::map<std::string, uint32_t> fProcessMap;

    std::set<std::string> fFilterProcess;
    std::set<int> fFilterParticle;
    double fFilterEnergy = -1;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

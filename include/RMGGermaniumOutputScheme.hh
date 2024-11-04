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

#ifndef _RMG_GERMANIUM_OUTPUT_SCHEME_HH_
#define _RMG_GERMANIUM_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGGermaniumDetector.hh"
#include "RMGVOutputScheme.hh"

class G4Event;
class RMGGermaniumOutputScheme : public RMGVOutputScheme {

  public:

    RMGGermaniumOutputScheme();

    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    bool ShouldDiscardEvent(const G4Event*) override;
    std::optional<bool> StackingActionNewStage(int) override;
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    inline void SetEdepCutLow(double threshold) { fEdepCutLow = threshold; }
    inline void SetEdepCutHigh(double threshold) { fEdepCutHigh = threshold; }
    inline void AddEdepCutDetector(int det_uid) { fEdepCutDetectors.insert(det_uid); }

  protected:

    [[nodiscard]] inline std::string GetNtuplenameFlat() const override { return "germanium"; }

  private:

    RMGGermaniumDetectorHitsCollection* GetHitColl(const G4Event*);

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    double fEdepCutLow = -1;
    double fEdepCutHigh = -1;
    std::set<int> fEdepCutDetectors;

    bool fDiscardPhotonsIfNoGermaniumEdep = false;

    bool fStoreSinglePrecisionEnergy = false;
    bool fStoreSinglePrecisionPosition = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

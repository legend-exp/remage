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

#ifndef _RMG_V_OUTPUT_SCHEME_HH_
#define _RMG_V_OUTPUT_SCHEME_HH_

#include <optional>
#include <string>

#include "G4AnalysisManager.hh"
#include "G4Track.hh"
#include "G4UserStackingAction.hh"

#include "fmt/format.h"

class G4Event;
class RMGVOutputScheme {

  public:

    RMGVOutputScheme() = default;

    // initialization.
    virtual inline void AssignOutputNames(G4AnalysisManager*) {}

    // functions for individual events.
    virtual inline void ClearBeforeEvent() {}
    virtual inline bool ShouldDiscardEvent(const G4Event*) { return false; }
    virtual inline void StoreEvent(const G4Event*) {}

    // hook into RMGStackingAction.
    virtual inline std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*,
        const int) {
      return std::nullopt;
    }
    virtual inline std::optional<bool> StackingActionNewStage(const int) { return std::nullopt; }

    inline std::string GetNtupleName(int det_uid) { return fmt::format("det{:03}", det_uid); }
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

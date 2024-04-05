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

#include <map>
#include <string>

#include "G4AnalysisManager.hh"

#include "fmt/format.h"

class G4Event;
class RMGVOutputScheme {

  public:

    inline RMGVOutputScheme(G4AnalysisManager* ana_man) { this->AssignOutputNames(ana_man); }
    virtual inline void clear() {};
    virtual inline void AssignOutputNames(G4AnalysisManager*) {};
    virtual inline void EndOfEventAction(const G4Event*) {};
    inline std::string GetNtupleName(int det_uid) { return fmt::format("det{:03}", det_uid); }
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

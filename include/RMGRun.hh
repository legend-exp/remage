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

#ifndef _RMG_RUN_HH_
#define _RMG_RUN_HH_

#include <chrono>

#include "G4Run.hh"

class RMGRun : public G4Run {

  public:

    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    [[nodiscard]] const TimePoint& GetStartTime() const { return fStartTime; }
    void SetStartTime(TimePoint t) { fStartTime = t; }

  private:

    TimePoint fStartTime;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

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

#ifndef _RMG_EXCEPTION_HANDLER_HH_
#define _RMG_EXCEPTION_HANDLER_HH_

#include <memory>
#include <vector>

#include "G4ExceptionHandler.hh"
#include "globals.hh"

#include "RMGLog.hh"

class G4VUserPhysicsList;
class RMGHardware;
class RMGUserAction;
class G4GenericMessenger;
class G4UIExecutive;
class RMGExceptionHandler : public G4ExceptionHandler {

  public:

    RMGExceptionHandler() = default;
    ~RMGExceptionHandler() = default;

    RMGExceptionHandler(const RMGExceptionHandler&) = delete;
    RMGExceptionHandler& operator=(const RMGExceptionHandler&) = delete;

    bool Notify(const char* originOfException, const char* exceptionCode,
        G4ExceptionSeverity severity, const char* description) override;

    [[nodiscard]] inline bool HadWarning() const { return fHadWarning; }
    [[nodiscard]] inline bool HadError() const { return fHadError; }

  private:

    bool fHadWarning = false;
    bool fHadError = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

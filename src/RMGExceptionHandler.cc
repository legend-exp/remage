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

#include "RMGExceptionHandler.hh"

#include "G4ExceptionHandler.hh"

bool RMGExceptionHandler::Notify(const char* originOfException, const char* exceptionCode,
    G4ExceptionSeverity severity, const char* description) {

  if (severity == JustWarning) {
    fHadWarning = true;
  } else {
    fHadError = true;
  }

  return G4ExceptionHandler::Notify(originOfException, exceptionCode, severity, description);
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#ifndef _RMG_EXCEPTION_HANDLER_HH_
#define _RMG_EXCEPTION_HANDLER_HH_

#include "G4ExceptionHandler.hh"

/**
 * @brief Geant4 exception handler that records whether warnings/errors have occurred.
 *
 * Forwards all exceptions to the standard Geant4 reporting path while keeping flags that
 * @ref RMGManager can query at the end of the job to set the process exit status.
 */
class RMGExceptionHandler : public G4ExceptionHandler {

  public:

    RMGExceptionHandler() = default;
    ~RMGExceptionHandler() = default;

    RMGExceptionHandler(const RMGExceptionHandler&) = delete;
    RMGExceptionHandler& operator=(const RMGExceptionHandler&) = delete;

    /**
     * @brief Record the severity and delegate the exception to @c G4ExceptionHandler.
     * @return The abort flag returned by the base implementation.
     */
    bool Notify(
        const char* originOfException,
        const char* exceptionCode,
        G4ExceptionSeverity severity,
        const char* description
    ) override;

    /** @brief Whether at least one warning-level exception has been raised. */
    [[nodiscard]] bool HadWarning() const { return fHadWarning; }
    /** @brief Whether at least one error-level (or higher) exception has been raised. */
    [[nodiscard]] bool HadError() const { return fHadError; }

  private:

    bool fHadWarning = false;
    bool fHadError = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

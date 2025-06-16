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


// Modified by Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571> in 2022

#ifndef _RMG_LOG_HH_
#define _RMG_LOG_HH_

/**
 * @class RMGLog
 * @brief A class for managing log messages.
 * @author Daniel Kollar
 * @author Kevin Kr&ouml;ninger
 * @version 1.0
 * @date 08.2008
 * @details This class manages log messages for printing on the screen
 */

/*
 * Copyright (C) 2007-2018, the BAT core developer team
 * All rights reserved.
 *
 * For the licensing terms see doc/COPYING.
 * For documentation see http://mpp.mpg.de/bat
 */

// ---------------------------------------------------------

#include <cstdarg>
#include <fstream>
#include <iostream>
#include <string>

#include "globals.hh"

#include "fmt/core.h"

#define OutDev(loglevel, ...) RMGLog::Out(loglevel, "[", __PRETTY_FUNCTION__, "] ", __VA_ARGS__)

#define OutFormatDev(loglevel, fmt, ...)                                                           \
  RMGLog::OutFormat(loglevel, "[" + std::string(__PRETTY_FUNCTION__) + "] " + fmt, __VA_ARGS__)

// ---------------------------------------------------------

class RMGLog {

  public:

    // definition of log level

    /**
     * Enumerator for the amount of details to put into the log */
    enum LogLevel {
      debug = 0,   ///< Print everything, including debug info
      detail = 1,  ///< Print all details of operation
      summary = 2, ///< Print only results summary, warnings, and errors
      warning = 3, ///< Print only warnings and errors
      error = 4,   ///< Print only errors
      fatal = 5,   ///< Print only errors, stop execution
      nothing = 6  ///< Print nothing
    };

    enum Ansi {
      black = 30,
      red = 31,
      green = 32,
      yellow = 33,
      blue = 34,
      magenta = 35,
      cyan = 36,
      grey = 37,
      unspecified
    };

    /** \name Constructor and destructor */
    /** @{ */

    /**
     * Constructor. */
    RMGLog();

    /** @} */
    /** \name Getters */
    /** @{ */

    /**
     * Returns the minimum log level for screen output.
     * @return log level */
    static RMGLog::LogLevel GetLogLevel() { return fMinimumLogLevel; }

    /**
     * Returns true if the loglevel is prefixed to every message.
     * Default: true */
    static bool GetPrefix() { return fUsePrefix; }

    /** @} */
    /** \name Setters */
    /** @{ */

    /**
     * Sets the minimum log level for screen output.
     * @param loglevel log level */
    static void SetLogLevel(RMGLog::LogLevel loglevel) { fMinimumLogLevel = loglevel; }

    /**
     * Toggle if the loglevel is prefixed to every message. */
    static void SetPrefix(bool flag) { fUsePrefix = flag; }

    /**
     * Toggle if a startup message is shown before the first log message. */
    static void SetInihibitStartupInfo(bool flag) { fFirstOutputDone = flag; }

    /** @} */
    /** \name Miscellaneous */
    /** @{ */

    /**
     * Writes string to the screen log if the log level is equal or greater than the minimum
     * @param loglevel loglevel for the current message
     * @param message string to write to the screen log */
    template<typename T> static void Out(RMGLog::LogLevel loglevel, const T& message);

    template<typename T, typename... Args>
    static void Out(RMGLog::LogLevel loglevel, const T& msg_first, const Args&... args);

    template<typename... Args>
    static void OutFormat(RMGLog::LogLevel loglevel, const std::string& fmt, const Args&... args);

    template<typename T, typename... Args>
    static void Out(RMGLog::LogLevel loglevel, T& msg_first, Args... msg_other) {
      RMGLog::Out(loglevel, msg_first, msg_other...);
    }

    /**
     * Writes startup information onto screen */
    static void StartupInfo();

    /**
     * @return string containing the version number  */
    static const std::string& GetVersion() { return fVersion; }

    static bool SupportsColors(const std::ostream& os);

    template<RMGLog::Ansi color, typename T>
    static std::string Colorize(const T& msg, std::ostream& os, bool bold = false);

    static bool HadWarning() { return fHadWarning; }
    static bool HadError() { return fHadError; }

    /** @} */

  private:

    template<typename T>
    static void Print(RMGLog::LogLevel loglevel, const T& msg, bool prefixed = true, bool do_flush = true);

    /**
     * Converts a log level to a string */
    static std::string GetPrefix(RMGLog::LogLevel, std::ostream& os);

    /**
     * BAT version number */
    static std::string fVersion;

    /**
     * The minimum screen log level */
    static RMGLog::LogLevel fMinimumLogLevel;

    /**
     * Specifies whether there were output printouts already */
    static bool fFirstOutputDone;

    /**
     * Specifies whether there was any warning logged yet */
    static bool fHadWarning;

    /**
     * Specifies whether there was any error logged yet */
    static bool fHadError;

    /**
     * Include a prefix before each message? */
    static bool fUsePrefix;
};

#include "RMGLog.icc"

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

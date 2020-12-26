#ifndef _RMGLOG_HH_
#define _RMGLOG_HH_

/**
 * @class RMGLog
 * @brief A class for managing log messages.
 * @author Daniel Kollar
 * @author Kevin Kr&ouml;ninger
 * @version 1.0
 * @date 08.2008
 * @details This class manages log messages for printing on the screen
 * and into a log file
 */

/*
 * Copyright (C) 2007-2018, the BAT core developer team
 * All rights reserved.
 *
 * For the licensing terms see doc/COPYING.
 * For documentation see http://mpp.mpg.de/bat
 */

// ---------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include <cstdarg>

// ---------------------------------------------------------

class RMGLog {

  public:

    // definition of log level

    /**
     * Enumerator for the amount of details to put into the log file */
    enum LogLevel {
      debug,   ///< Print everything, including debug info
      detail,  ///< Print all details of operation
      summary, ///< Print only results summary, warnings, and errors
      warning, ///< Print only warnings and errors
      error,   ///< Print only errors
      fatal,   ///< Print only errors, stop execution
      nothing  ///< Print nothing
    };

    enum Ansi {
      black   = 30,
      red     = 31,
      green   = 32,
      yellow  = 33,
      blue    = 34,
      magenta = 35,
      cyan    = 36,
      grey    = 37
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
     * Returns the minimum log level for file output.
     * @return log level */
    static inline RMGLog::LogLevel GetLogLevelFile() { return fMinimumLogLevelFile; }

    /**
     * Returns the minimum log level for screen output.
     * @return log level */
    static inline RMGLog::LogLevel GetLogLevelScreen() { return fMinimumLogLevelScreen; }

    /**
     * Returns true if the loglevel is prefixed to every message.
     * Default: true */
    static inline bool GetPrefix() { return fUsePrefix; }

    /** @} */
    /** \name Setters */
    /** @{ */

    /**
     * Sets the minimum log level for file output.
     * @param loglevel log level */
    static inline void SetLogLevelFile(RMGLog::LogLevel loglevel) { fMinimumLogLevelFile = loglevel; }

    /**
     * Sets the minimum log level for screen output.
     * @param loglevel log level */
    static inline void SetLogLevelScreen(RMGLog::LogLevel loglevel) { fMinimumLogLevelScreen = loglevel; }

    /**
     * Sets the minimum log level for file and screen output.
     * @param loglevelfile log level for file
     * @param loglevelscreen log level for screen */
    static inline void SetLogLevel(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen) {
      fMinimumLogLevelFile = loglevelfile; fMinimumLogLevelScreen = loglevelscreen;
    }

    /**
     * Sets the minimum log level for file and screen output.
     * @param loglevel log level */
    static inline void SetLogLevel(RMGLog::LogLevel loglevel) { SetLogLevel(loglevel, loglevel); }

    /**
     * Toggle if the loglevel is prefixed to every message. */
    static inline void SetPrefix(bool flag) { fUsePrefix = flag; }

    /** @} */
    /** \name Miscellaneous */
    /** @{ */

    /**
     * Opens log file and sets minimum log levels for file and screen output.
     * @param filename log filename
     * @param loglevelfile minimum log level for file output
     * @param loglevelscreen minimum log level for screen output */
    static void OpenLogFile(const std::string& filename);

    /**
     * @returns true if log file is open or false if not. */
    static inline bool IsOpen() { return fOutputFileStream.is_open(); }

    /**
     * Closes the log file */
    static inline void CloseLog() { fOutputFileStream.close(); }

    /**
     * Writes string to the file and screen log if the log level is equal or greater than the minimum
     * @param loglevelfile loglevel for the current message
     * @param loglevelscreen loglevel for the current message
     * @param message string to write to the file and screen log */
    template <typename T>
    static void Out(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, const T& msg);

    template <typename T, typename... Args>
    static void Out(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, const T& msg_first, const Args&... msg_other);

    static void OutFormat(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, const char *fmt, ...);

    static void OutFormat(RMGLog::LogLevel loglevel, const char *fmt, ...);

    template <typename T>
    static inline void Out(RMGLog::LogLevel loglevel, const T& msg) { RMGLog::Out(loglevel, loglevel, msg); }

    template <typename T, typename... Args>
    static inline void Out(RMGLog::LogLevel loglevel, T& msg_first, Args... msg_other) {
        RMGLog::Out(loglevel, loglevel, msg_first, msg_other...);
    }

    /**
     * Writes startup information onto screen and into a logfile */
    static void StartupInfo();

    /**
     * @return string containing the version number  */
    static inline const std::string& GetVersion() { return fVersion; }

    static bool SupportsColors(const std::ostream& os);

    /** @} */
  private:

    template <typename T>
    static void Print(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, const T& msg, bool prefixed=true, bool do_flush=true);

    /**
     * Converts a log level to a string
     * @param force_no_colors forcibly disable usage of ANSI escape sequences (e.g. if printing to file) */
    static std::string GetPrefix(RMGLog::LogLevel, std::ostream& os);

    template <RMGLog::Ansi color, typename T>
    static std::string Colorize(const T& msg, std::ostream& os, bool bold=false);

    /**
     * BAT version number */
    static std::string fVersion;

    /**
     * The minimum file log level */
    static RMGLog::LogLevel fMinimumLogLevelFile;

    /**
     * The minimum screen log level */
    static RMGLog::LogLevel fMinimumLogLevelScreen;

    /**
     * The output stream for the file log */
    static std::ofstream fOutputFileStream;

    /**
     * Specifies whether there were output printouts already */
    static bool fFirstOutputDone;

    /**
     * Include a prefix before each message? */
    static bool fUsePrefix;

};

#include "RMGLog.icc"

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

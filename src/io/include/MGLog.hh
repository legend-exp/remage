#ifndef __MGLOG_HH
#define __MGLOG_HH

/**
 * @class MGLog
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

#include <fstream>
#include <string>
#include <cstdio>
#include <cstdarg>

// ---------------------------------------------------------

class MGLog {

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
      gray    = 37
    };

    /** \name Constructor and destructor */
    /** @{ */

    /**
     * Constructor. */
    MGLog();

    /** @} */
    /** \name Getters */
    /** @{ */

    /**
     * Returns the minimum log level for file output.
     * @return log level */
    static inline MGLog::LogLevel GetLogLevelFile() { return fMinimumLogLevelFile; }

    /**
     * Returns the minimum log level for screen output.
     * @return log level */
    static inline MGLog::LogLevel GetLogLevelScreen() { return fMinimumLogLevelScreen; }

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
    static inline void SetLogLevelFile(MGLog::LogLevel loglevel) { fMinimumLogLevelFile = loglevel; }

    /**
     * Sets the minimum log level for screen output.
     * @param loglevel log level */
    static inline void SetLogLevelScreen(MGLog::LogLevel loglevel) { fMinimumLogLevelScreen = loglevel; }

    /**
     * Sets the minimum log level for file and screen output.
     * @param loglevelfile log level for file
     * @param loglevelscreen log level for screen */
    static inline void SetLogLevel(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen) {
      fMinimumLogLevelFile = loglevelfile; fMinimumLogLevelScreen = loglevelscreen;
    }

    /**
     * Sets the minimum log level for file and screen output.
     * @param loglevel log level */
    static inline void SetLogLevel(MGLog::LogLevel loglevel) { SetLogLevel(loglevel, loglevel); }

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
    static void OpenLog(const std::string& filename = "log.txt", MGLog::LogLevel loglevelfile = MGLog::debug, MGLog::LogLevel loglevelscreen = MGLog::summary);

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
    static void Out(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, const T& msg);

    template <typename T, typename... Args>
    static void Out(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, T& msg_first, Args... msg_other);

    static void OutFormat(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, const char *fmt, ...);

    static void OutFormat(MGLog::LogLevel loglevel, const char *fmt, ...);

    template <typename T>
    static inline void Out(MGLog::LogLevel loglevel, const T& msg) { Out(loglevel, loglevel, msg); }

    template <typename T, typename... Args>
    static inline void Out(MGLog::LogLevel loglevel, T& msg_first, Args... msg_other) { Out(loglevel, loglevel, msg_first, msg_other...); }

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
    static void Print(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, T& t, bool prefixed=true, bool do_flush=true);

    /**
     * Converts a log level to a string
     * @param force_no_colors forcibly disable usage of ANSI escape sequences (e.g. if printing to file) */
    static std::string GetPrefix(MGLog::LogLevel, std::ostream& os);

    template <MGLog::Ansi color, typename T>
    static std::string Colorize(const T& msg, std::ostream& os, bool bold=false);

    /**
     * BAT version number */
    static std::string fVersion;

    /**
     * The minimum file log level */
    static MGLog::LogLevel fMinimumLogLevelFile;

    /**
     * The minimum screen log level */
    static MGLog::LogLevel fMinimumLogLevelScreen;

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

// ---------------------------------------------------------

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

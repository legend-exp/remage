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

/**
 * Macros to output a message to the log including the class and method name.
 */
#define MGLOG_INTERNAL_OUT(t, s) \
    do {\
    MGLog:: t \
      (std::string(__PRETTY_FUNCTION__) + ": " + s); \
    } while (false)

#define MGLOG_DEBUG(s)   MGLOG_INTERNAL_OUT(OutDebug, s)

#define MGLOG_DETAIL(s)  MGLOG_INTERNAL_OUT(OutDetail, s)

#define MGLOG_ERROR(s)   MGLOG_INTERNAL_OUT(OutError, s)

#define MGLOG_SUMMARY(s) MGLOG_INTERNAL_OUT(OutSummary, s)

#define MGLOG_WARNING(s) MGLOG_INTERNAL_OUT(OutWarning, s)

// ---------------------------------------------------------

#include <fstream>
#include <string>

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
    static inline MGLog::LogLevel GetLogLevelFile() { return fMinimumLogLevelFile; };

    /**
     * Returns the minimum log level for screen output.
     * @return log level */
    static inline MGLog::LogLevel GetLogLevelScreen() { return fMinimumLogLevelScreen; };

    /**
     * Returns true if the loglevel is prefixed to every message.
     * Default: true */
    static inline bool GetPrefix() { return fPrefix; }

    /** @} */
    /** \name Setters */
    /** @{ */

    /**
     * Use ANSI escape sequences to color the output on the terminal (only). */
    static inline bool SetColoredOutput(bool flag) { fColoredOutput = flag; }

    /**
     * Sets the minimum log level for file output.
     * @param loglevel log level */
    static inline void SetLogLevelFile(MGLog::LogLevel loglevel) { fMinimumLogLevelFile = loglevel; };

    /**
     * Sets the minimum log level for screen output.
     * @param loglevel log level */
    static inline void SetLogLevelScreen(MGLog::LogLevel loglevel) { fMinimumLogLevelScreen = loglevel; };

    /**
     * Sets the minimum log level for file and screen output.
     * @param loglevelfile log level for file
     * @param loglevelscreen log level for screen */
    static inline void SetLogLevel(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen) {
      fMinimumLogLevelFile = loglevelfile; fMinimumLogLevelScreen = loglevelscreen;
    };

    /**
     * Sets the minimum log level for file and screen output.
     * @param loglevel log level */
    static inline void SetLogLevel(MGLog::LogLevel loglevel) { SetLogLevel(loglevel, loglevel); };

    /**
     * Toggle if the loglevel is prefixed to every message. */
    static inline void SetPrefix(bool flag) { fPrefix = flag; }

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
    static inline bool IsOpen() { return fOutputStream.is_open(); }

    /**
     * Closes the log file */
    static inline void CloseLog() { fOutputStream.close(); }

    /**
     * Writes string to the file and screen log if the log level is equal or greater than the minimum
     * @param loglevelfile loglevel for the current message
     * @param loglevelscreen loglevel for the current message
     * @param message string to write to the file and screen log */
    static void Out(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, const std::string& message);

    static inline void Out(const std::string& message) { Out(MGLog::fMinimumLogLevelFile, MGLog::fMinimumLogLevelScreen, message); }

    static inline void Out(MGLog::LogLevel loglevel, const std::string& message) { Out(loglevel, loglevel, message); };

    static inline void OutFatal(const std::string& message) { Out(fatal, message); };

    static inline void OutError(const std::string& message) { Out(error, message); };

    static inline void OutWarning(const std::string& message) { Out(warning, message); };

    static inline void OutSummary(const std::string& message) { Out(summary, message); };

    static inline void OutDetail(const std::string& message) { Out(detail, message); };

    static inline void OutDebug(const std::string& message) { Out(debug, message); };

    /**
     * Writes startup information onto screen and into a logfile */
    static void StartupInfo();

    /**
     * @return string containing the version number  */
    static inline const std::string& GetVersion() { return fVersion; };

    /**
     * Converts a log level to a string
     * @param force_no_colors forcibly disable usage of ANSI escape sequences (e.g. if printing to file) */
    static std::string ToString(MGLog::LogLevel, bool force_no_colors=false);

    /** @} */
  private:

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
    static std::ofstream fOutputStream;

    /**
     * Specifies whether there were output printouts already */
    static bool fFirstOutputDone;

    /**
     * Include a prefix before each message? */
    static bool fPrefix;

    /**
     * Use colors on the terminal? */
    static bool fColoredOutput;

};

// ---------------------------------------------------------

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

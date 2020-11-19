/*
 * Copyright (C) 2007-2018, the BAT core developer team
 * All rights reserved.
 *
 * For the licensing terms see doc/COPYING.
 * For documentation see http://mpp.mpg.de/bat
 */

// ---------------------------------------------------------

#include "MGLog.hh"
#include "ProjectInfo.hh"

#include <TError.h>
#include <TROOT.h>

#include <iostream>
#include <iomanip>

std::ofstream MGLog::fOutputStream;

MGLog::LogLevel MGLog::fMinimumLogLevelFile = MGLog::debug;

MGLog::LogLevel MGLog::fMinimumLogLevelScreen = MGLog::summary;

bool MGLog::fFirstOutputDone = false;

bool MGLog::fPrefix = true;

bool MGLog::fColoredOutput = true;

std::string MGLog::fVersion = PROJECT_VERSION;

// ---------------------------------------------------------

MGLog::MGLog() {
    // suppress the ROOT Info printouts
    gErrorIgnoreLevel = 2000;
}

// ---------------------------------------------------------

void MGLog::OpenLog(const std::string& filename, MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen) {

    // suppress the ROOT Info printouts
    gErrorIgnoreLevel = 2000;

    // first close and flush and existing log file
    CloseLog();

    // open log file
    MGLog::fOutputStream.open(filename.data());

    if (!MGLog::fOutputStream.is_open()) {
        std::cerr << " Could not open log file " << filename << ". " << std::endl;
        return;
    }

    // set log level
    MGLog::SetLogLevelFile(loglevelfile);
    MGLog::SetLogLevelScreen(loglevelscreen);

    MGLog::Out(MGLog::summary, MGLog::summary, "Opening logfile " + filename);
}

// ---------------------------------------------------------

void MGLog::Out(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, const std::string& message) {
    // if this is the first call to Out(), call StartupInfo() first
    if (!MGLog::fFirstOutputDone)
        MGLog::StartupInfo();

    // open log file if not opened
    if (MGLog::IsOpen()) {
        // write message in to log file
        if (loglevelfile >= MGLog::fMinimumLogLevelFile)
            MGLog::fOutputStream << MGLog::ToString(loglevelfile, true) << message << std::endl;
    }

    // write message to screen
    if (loglevelscreen >= MGLog::fMinimumLogLevelScreen)
        std::cout << MGLog::ToString(loglevelscreen) << message << std::endl;

    // thorw exception if error is fatal
    if (loglevelfile == fatal or loglevelscreen == fatal) {
      throw std::runtime_error("A fatal exception has occurred, the execution cannot continue.");
    }
}

// ---------------------------------------------------------

void MGLog::StartupInfo() {
    const char* message = Form(
                              " +------------------------------------------------------+\n"
                              " |                                                      |\n"
                              " | remage version %-12s                          |\n"
                              " |                                                      |\n"
                              " +------------------------------------------------------+\n",
                              MGLog::fVersion.data());

    // write message to screen
    if (MGLog::fMinimumLogLevelScreen < MGLog::nothing)
        std::cout << message << std::endl;

    if (MGLog::IsOpen() && MGLog::fMinimumLogLevelFile < MGLog::nothing)
        MGLog::fOutputStream << message;

    fFirstOutputDone = true;
}

// ---------------------------------------------------------

std::string MGLog::ToString(MGLog::LogLevel loglevel, bool force_no_colors) {

    if (!fPrefix) return "";

    if (MGLog::fColoredOutput and !force_no_colors) {
      switch (loglevel) {
          case debug:
              return   "\033[35m[Debug ___\033[m ";
          case detail:
              return   "\033[34m[Detail __\033[m ";
          case summary:
              return   "\033[32m[Summary _\033[m ";
          case warning:
              return "\033[1;33m[Warning _\033[m ";
          case error:
              return "\033[1;31m[Error ___\033[m ";
          case fatal:
              return "\033[1;31m[Fatal ___\033[m ";
          default:
              return "";
      }
    }
    else {
      switch (loglevel) {
          case debug:
              return "[Debug ___ ";
          case detail:
              return "[Detail __ ";
          case summary:
              return "[Summary _ ";
          case warning:
              return "[Warning _ ";
          case error:
              return "[Error ___ ";
          case fatal:
              return "[Fatal ___ ";
          default:
              return "";
      }
    }
}

// vim: tabstop=2 shiftwidth=2 expandtab

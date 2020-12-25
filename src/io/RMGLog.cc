/*
 * Copyright (C) 2007-2018, the BAT core developer team
 * All rights reserved.
 *
 * For the licensing terms see doc/COPYING.
 * For documentation see http://mpp.mpg.de/bat
 */

// ---------------------------------------------------------

#include "RMGLog.hh"
#include "ProjectInfo.hh"

#if RMG_HAS_ROOT
#include <TError.h>
#include <TROOT.h>
#endif

#include <iomanip>
#include <memory>
#include <unistd.h> // for isatty()
#include <cstring>
#include <algorithm>

std::ofstream RMGLog::fOutputFileStream;

RMGLog::LogLevel RMGLog::fMinimumLogLevelFile = RMGLog::debug;

RMGLog::LogLevel RMGLog::fMinimumLogLevelScreen = RMGLog::summary;

bool RMGLog::fFirstOutputDone = false;

bool RMGLog::fUsePrefix = true;

std::string RMGLog::fVersion = RMG_PROJECT_VERSION;

// initialize them at start of program - mandatory
// so that even if user redirects, we've a copy
std::streambuf const *coutbuf = std::cout.rdbuf();
std::streambuf const *cerrbuf = std::cerr.rdbuf();
std::streambuf const *clogbuf = std::clog.rdbuf();

// ---------------------------------------------------------

RMGLog::RMGLog() {
#if RMG_HAS_ROOT
    // suppress the ROOT Info printouts
    gErrorIgnoreLevel = 2000;
#endif
}

// ---------------------------------------------------------

void RMGLog::OpenLogFile(const std::string& filename) {

#if RMG_HAS_ROOT
    // suppress the ROOT Info printouts
    gErrorIgnoreLevel = 2000;
#endif

    // first close and flush and existing log file
    CloseLog();

    // open log file
    RMGLog::fOutputFileStream.open(filename.data());

    if (!RMGLog::fOutputFileStream.is_open()) {
        std::cerr << " Could not open log file " << filename << ". " << std::endl;
        return;
    }

    RMGLog::Out(RMGLog::summary, RMGLog::summary, "Opening logfile " + filename);
}

// ---------------------------------------------------------

void RMGLog::StartupInfo() {

    std::string message = "";
    message += " >>>\n";
    message += " >>> remage version: " + RMGLog::fVersion + "\n";
    message += " >>>\n";

    // write message to screen
    if (RMGLog::fMinimumLogLevelScreen < RMGLog::nothing)
        std::cout << message << std::endl;

    if (RMGLog::IsOpen() && RMGLog::fMinimumLogLevelFile < RMGLog::nothing)
        RMGLog::fOutputFileStream << message;

    fFirstOutputDone = true;
}

// ---------------------------------------------------------

std::string RMGLog::GetPrefix(RMGLog::LogLevel loglevel, std::ostream& os) {

    if (!fUsePrefix) return "";

    switch (loglevel) {
        case debug:
            return Colorize<RMGLog::Ansi::magenta>("[Debug ___ ", os);
        case detail:
            return Colorize<RMGLog::Ansi::blue>   ("[Detail __ ", os);
        case summary:
            return Colorize<RMGLog::Ansi::green>  ("[Summary _ ", os);
        case warning:
            return Colorize<RMGLog::Ansi::yellow> ("[Warning _ ", os);
        case error:
            return Colorize<RMGLog::Ansi::red>    ("[Error ___ ", os);
        case fatal:
            return Colorize<RMGLog::Ansi::red>    ("[Fatal ___ ", os, true);
        default:
            return "";
    }
}

// https://github.com/agauniyal/rang/blob/master/include/rang.hpp
bool RMGLog::SupportsColors(const std::ostream& os) {

  // determine whether the stream refers to a file or a screen
  auto osbuf = os.rdbuf();
  FILE* the_stream = nullptr;
  if (osbuf == coutbuf) {
    the_stream = stdout;
  }
  else if (osbuf == cerrbuf or osbuf == clogbuf) {
    the_stream = stderr;
  }
  else return false;

  // check that we are on a tty
  if (!::isatty(::fileno(the_stream))) return false;

  // check the value of the TERM variable
  const char* terms[] = {
    "ansi", "color", "console", "cygwin", "gnome",
    "konsole", "kterm", "linux", "msys", "putty",
    "rxvt", "screen", "vt100", "xterm"
  };

  auto env_p = std::getenv("TERM");
  if (env_p == nullptr) return false;

  return std::any_of(std::begin(terms), std::end(terms),
    [&](const char *term) { return ::strstr(env_p, term) != nullptr; });
}

// vim: tabstop=2 shiftwidth=2 expandtab

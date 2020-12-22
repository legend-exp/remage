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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <unistd.h> // for isatty()

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

template <typename T>
void RMGLog::Out(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, const T& message) {
  // if this is the first call to Out(), call StartupInfo() first
  if (!RMGLog::fFirstOutputDone) RMGLog::StartupInfo();

  RMGLog::Print(loglevelfile, loglevelscreen, message, true);

  // thorw exception if error is fatal
  if (loglevelfile == fatal or loglevelscreen == fatal) {
    throw std::runtime_error("A fatal exception has occurred, the execution cannot continue.");
  }
}

// ---------------------------------------------------------

template <typename T, typename... Args>
void RMGLog::Out(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, T& t, Args... args) {

  // if this is the first call to Out(), call StartupInfo() first
  if (!RMGLog::fFirstOutputDone) RMGLog::StartupInfo();

  RMGLog::Print(loglevelfile, loglevelscreen, t,       true,  false);
  RMGLog::Print(loglevelfile, loglevelscreen, args..., false, false);
  RMGLog::Print(loglevelfile, loglevelscreen, "\n",    false);

  // thorw exception if error is fatal
  if (loglevelfile == fatal or loglevelscreen == fatal) {
    throw std::runtime_error("A fatal exception has occurred, the execution cannot continue.");
  }
}

// ---------------------------------------------------------

// https://codereview.stackexchange.com/questions/187183/create-a-c-string-using-printf-style-formatting
void RMGLog::OutFormat(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, const char *fmt, ...) {

  char buf[256];
  va_list args;
  va_start(args, fmt);
  const auto r = std::vsnprintf(buf, sizeof buf, fmt, args);
  va_end(args);

  // conversion failed
  if (r < 0) {
    RMGLog::Out(RMGLog::error, "Formatting error");
    return;
  }

  // we fit in the buffer
  const size_t len = r;
  if (len < sizeof buf) {
    RMGLog::Out(loglevelfile, loglevelscreen, std::string{buf, len});
    return;
  }

  // we need to allocate scratch memory
  auto vbuf = std::unique_ptr<char[]>(new char[len+1]);
  va_start(args, fmt);
  std::vsnprintf(vbuf.get(), len+1, fmt, args);
  va_end(args);
  RMGLog::Out(loglevelfile, loglevelscreen, std::string{vbuf.get(), len});
}

// ---------------------------------------------------------

void OutFormat(RMGLog::LogLevel loglevel, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  RMGLog::OutFormat(loglevel, loglevel, fmt, args);
  va_end(args);
}

// ---------------------------------------------------------

template <typename T>
void RMGLog::Print(RMGLog::LogLevel loglevelfile, RMGLog::LogLevel loglevelscreen, T& msg, bool prefixed, bool do_flush) {

  // open log file if not opened
  if (RMGLog::IsOpen()) {
    // write message in to log file
    if (loglevelfile >= RMGLog::fMinimumLogLevelFile) {
      if (prefixed) RMGLog::fOutputFileStream << RMGLog::GetPrefix(loglevelfile, RMGLog::fOutputFileStream);
      RMGLog::fOutputFileStream << msg;
      if (do_flush) RMGLog::fOutputFileStream << std::flush;
    }
  }

  // write message to screen
  if (loglevelscreen >= RMGLog::fMinimumLogLevelScreen) {
    std::ostream& strm = loglevelscreen > RMGLog::LogLevel::warning ? std::cout : std::cerr;
    if (prefixed) strm << RMGLog::GetPrefix(loglevelscreen, strm);
    strm << msg;
    if (do_flush) strm << std::flush;
  }
}

// ---------------------------------------------------------

void RMGLog::StartupInfo() {
    const char* message = Form(
                              " +------------------------------------------------------+\n"
                              " |                                                      |\n"
                              " | remage version %-12s                          |\n"
                              " |                                                      |\n"
                              " +------------------------------------------------------+\n",
                              RMGLog::fVersion.data());

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

template <RMGLog::Ansi color, typename T>
static std::string Colorize(const T& msg, std::ostream& os, bool bold) {

  // check terminal capabilities before
  if (!RMGLog::SupportsColors(os)) return msg;

  std::ostringstream ss;
  ss << "\033[" << (bold ? "1;" : "") << color << "m" << msg << "\033[0m";
  return ss.str();
}

// vim: tabstop=2 shiftwidth=2 expandtab

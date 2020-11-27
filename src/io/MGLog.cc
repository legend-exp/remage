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
#include <sstream>
#include <memory>

std::ofstream MGLog::fOutputFileStream;

MGLog::LogLevel MGLog::fMinimumLogLevelFile = MGLog::debug;

MGLog::LogLevel MGLog::fMinimumLogLevelScreen = MGLog::summary;

bool MGLog::fFirstOutputDone = false;

bool MGLog::fUsePrefix = true;

std::string MGLog::fVersion = PROJECT_VERSION;

// initialize them at start of program - mandatory
// so that even if user redirects, we've a copy
std::streambuf const *coutbuf = std::cout.rdbuf();
std::streambuf const *cerrbuf = std::cerr.rdbuf();
std::streambuf const *clogbuf = std::clog.rdbuf();

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
    MGLog::fOutputFileStream.open(filename.data());

    if (!MGLog::fOutputFileStream.is_open()) {
        std::cerr << " Could not open log file " << filename << ". " << std::endl;
        return;
    }

    // set log level
    MGLog::SetLogLevelFile(loglevelfile);
    MGLog::SetLogLevelScreen(loglevelscreen);

    MGLog::Out(MGLog::summary, MGLog::summary, "Opening logfile " + filename);
}

// ---------------------------------------------------------

template <typename T>
void MGLog::Out(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, const T& message) {
  // if this is the first call to Out(), call StartupInfo() first
  if (!MGLog::fFirstOutputDone) MGLog::StartupInfo();

  MGLog::Print(loglevelfile, loglevelscreen, message, true);

  // thorw exception if error is fatal
  if (loglevelfile == fatal or loglevelscreen == fatal) {
    throw std::runtime_error("A fatal exception has occurred, the execution cannot continue.");
  }
}

// ---------------------------------------------------------

template <typename T, typename... Args>
void MGLog::Out(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, T& t, Args... args) {

  // if this is the first call to Out(), call StartupInfo() first
  if (!MGLog::fFirstOutputDone) MGLog::StartupInfo();

  MGLog::Print(loglevelfile, loglevelscreen, t,       true,  false);
  MGLog::Print(loglevelfile, loglevelscreen, args..., false, false);
  MGLog::Print(loglevelfile, loglevelscreen, "\n",    false);

  // thorw exception if error is fatal
  if (loglevelfile == fatal or loglevelscreen == fatal) {
    throw std::runtime_error("A fatal exception has occurred, the execution cannot continue.");
  }
}

// ---------------------------------------------------------

// https://codereview.stackexchange.com/questions/187183/create-a-c-string-using-printf-style-formatting
void MGLog::OutFormat(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, const char *fmt, ...) {

  char buf[256];
  va_list args;
  va_start(args, fmt);
  const auto r = std::vsnprintf(buf, sizeof buf, fmt, args);
  va_end(args);

  // conversion failed
  if (r < 0) {
    MGLog::Out(MGLog::error, "Formatting error");
    return;
  }

  // we fit in the buffer
  const size_t len = r;
  if (len < sizeof buf) {
    MGLog::Out(loglevelfile, loglevelscreen, std::string{buf, len});
    return;
  }

  // we need to allocate scratch memory
  auto vbuf = std::unique_ptr<char[]>(new char[len+1]);
  va_start(args, fmt);
  std::vsnprintf(vbuf.get(), len+1, fmt, args);
  va_end(args);
  MGLog::Out(loglevelfile, loglevelscreen, std::string{vbuf.get(), len});
}

// ---------------------------------------------------------

void OutFormat(MGLog::LogLevel loglevel, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  MGLog::OutFormat(loglevel, loglevel, fmt, args);
  va_end(args);
}

// ---------------------------------------------------------

template <typename T>
void MGLog::Print(MGLog::LogLevel loglevelfile, MGLog::LogLevel loglevelscreen, T& msg, bool prefixed, bool do_flush) {

  // open log file if not opened
  if (MGLog::IsOpen()) {
    // write message in to log file
    if (loglevelfile >= MGLog::fMinimumLogLevelFile) {
      if (prefixed) MGLog::fOutputFileStream << MGLog::GetPrefix(loglevelfile, MGLog::fOutputFileStream);
      MGLog::fOutputFileStream << msg;
      if (do_flush) MGLog::fOutputFileStream << std::flush;
    }
  }

  // write message to screen
  if (loglevelscreen >= MGLog::fMinimumLogLevelScreen) {
    std::ostream& strm = loglevelscreen > MGLog::LogLevel::warning ? std::cout : std::cerr;
    if (prefixed) strm << MGLog::GetPrefix(loglevelscreen, strm);
    strm << msg;
    if (do_flush) strm << std::flush;
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
        MGLog::fOutputFileStream << message;

    fFirstOutputDone = true;
}

// ---------------------------------------------------------

std::string MGLog::GetPrefix(MGLog::LogLevel loglevel, std::ostream& os) {

    if (!fUsePrefix) return "";

    switch (loglevel) {
        case debug:
            return Colorize<MGLog::Ansi::magenta>("[Debug ___ ", os);
        case detail:
            return Colorize<MGLog::Ansi::blue>   ("[Detail __ ", os);
        case summary:
            return Colorize<MGLog::Ansi::green>  ("[Summary _ ", os);
        case warning:
            return Colorize<MGLog::Ansi::yellow> ("[Warning _ ", os);
        case error:
            return Colorize<MGLog::Ansi::red>    ("[Error ___ ", os);
        case fatal:
            return Colorize<MGLog::Ansi::red>    ("[Fatal ___ ", os, true);
        default:
            return "";
    }
}

// https://github.com/agauniyal/rang/blob/master/include/rang.hpp
bool MGLog::SupportsColors(const std::ostream& os) {

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

template <MGLog::Ansi color, typename T>
static std::string Colorize(const T& msg, std::ostream& os, bool bold) {

  // check terminal capabilities before
  if (!MGLog::SupportsColors(os)) return msg;

  std::ostringstream ss;
  ss << "\033[" << (bold ? "1;" : "") << color << "m" << msg << "\033[0m";
  return ss.str();
}

// vim: tabstop=2 shiftwidth=2 expandtab

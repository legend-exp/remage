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
//
// Original copyright statement:
/*
 * Copyright (C) 2007-2018, the BAT core developer team
 * All rights reserved.
 *
 * For the licensing terms see doc/COPYING.
 * For documentation see http://mpp.mpg.de/bat
 */

// ---------------------------------------------------------

#include "RMGLog.hh"

#include "G4Version.hh"

#include "RMGConfig.hh"
#include "RMGVersion.hh"

#if RMG_HAS_ROOT
#include <TError.h>
#include <TROOT.h>
#endif

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iomanip>
#include <memory>
#include <regex>
#include <string>
#include <unistd.h> // for isatty()
#include <vector>

RMGLog::LogLevel RMGLog::fMinimumLogLevel = RMGLog::summary;

bool RMGLog::fFirstOutputDone = false;
bool RMGLog::fHadWarning = false;
bool RMGLog::fHadError = false;

bool RMGLog::fUsePrefix = true;
int RMGLog::fProcNum = -1;

std::string RMGLog::fVersion = RMG_PROJECT_VERSION_FULL;

// initialize them at start of program - mandatory
// so that even if user redirects, we've got a copy
/// \cond this triggers a sphinx error
std::streambuf const* coutbuf = G4cout.rdbuf();
std::streambuf const* cerrbuf = G4cerr.rdbuf();
/// \endcond

// ---------------------------------------------------------

RMGLog::RMGLog() {

#if RMG_HAS_ROOT
  // suppress the ROOT Info printouts
  gErrorIgnoreLevel = 2000;
#endif
}

// ---------------------------------------------------------

void RMGLog::StartupInfo() {

  auto g4_version = std::regex_replace(G4Version, std::regex("\\$|Name:"), "");

  std::string message;
  // clang-format off
  message += R"(  _ __ ___ _ __ ___   __ _  __ _  ___ )" "\n";
  message += R"( | '__/ _ \ '_ ` _ \ / _` |/ _` |/ _ \)" "\n";
  message += R"( | | |  __/ | | | | | (_| | (_| |  __/ v)" + RMGLog::fVersion + "\n";
  message += R"( |_|  \___|_| |_| |_|\__,_|\__, |\___| using)" + g4_version + "\n";
  message += R"(                           |___/      )" "\n";
  // clang-format on

  // write message to screen
  if (RMGLog::fMinimumLogLevel <= RMGLog::summary) G4cout << message << G4endl;

  fFirstOutputDone = true;
}

// ---------------------------------------------------------

std::string RMGLog::GetPrefix(RMGLog::LogLevel loglevel, std::ostream& os) {

  if (!fUsePrefix) return "";

  std::string proc_prefix;
  if (fProcNum >= 0) proc_prefix = "<p" + std::to_string(fProcNum) + "> ";

  switch (loglevel) {
    case debug: return Colorize<RMGLog::Ansi::magenta>(proc_prefix + "[Debug ---> ", os);
    case detail: return Colorize<RMGLog::Ansi::blue>(proc_prefix + "[Detail --> ", os);
    case summary: return Colorize<RMGLog::Ansi::green>(proc_prefix + "[Summary -> ", os);
    case warning: return Colorize<RMGLog::Ansi::yellow>(proc_prefix + "[Warning -> ", os);
    case error: return Colorize<RMGLog::Ansi::red>(proc_prefix + "[Error ---> ", os);
    case fatal: return Colorize<RMGLog::Ansi::red>(proc_prefix + "[Fatal ---> ", os, true);
    default: return "";
  }
}

// https://github.com/agauniyal/rang/blob/master/include/rang.hpp
bool RMGLog::SupportsColors(const std::ostream& os) {

  // determine whether the stream refers to a file or a screen
  auto osbuf = os.rdbuf();
  FILE* the_stream = nullptr;
  if (osbuf == coutbuf) {
    the_stream = stdout;
  } else if (osbuf == cerrbuf) {
    the_stream = stderr;
  } else return false;

  // check that we are on a tty
  if (!::isatty(::fileno(the_stream))) return false;

  // check the value of the TERM variable
  const std::vector<std::string> terms =
      {"ansi",
       "color",
       "console",
       "cygwin",
       "gnome",
       "konsole",
       "kterm",
       "linux",
       "msys",
       "putty",
       "rxvt",
       "screen",
       "vt100",
       "xterm"};

  auto env_p = std::getenv("TERM");
  if (env_p == nullptr) return false;
  std::string env_s{env_p};

  return std::any_of(std::begin(terms), std::end(terms), [&](const auto term) {
    return env_s.find(term) != std::string::npos;
  });
}

// vim: tabstop=2 shiftwidth=2 expandtab

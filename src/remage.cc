// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
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

#include <string>
#include <vector>

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include "fmt/core.h"
#include "magic_enum/magic_enum.hpp"

namespace CLI {
  namespace detail {
    bool lexical_cast(std::string input, RMGLog::LogLevel& output) {
      try {
        output = static_cast<RMGLog::LogLevel>(std::stoll(input));
        return true;
      } catch (...) {
        auto r = magic_enum::enum_cast<RMGLog::LogLevel>(input);
        if (r.has_value()) output = r.value();
        return r.has_value();
      }
    }
  } // namespace detail
} // namespace CLI

#include "CLI11/CLI11.hpp"

int main(int argc, char** argv) {

  CLI::App app{"remage: simulation framework for germanium experiments"};

  int verbosity;
  bool quiet = false;
  int nthreads = 1;
  bool interactive = false;
  std::vector<std::string> gdmls;
  std::vector<std::string> macros;
  std::string output;
  RMGLog::LogLevel loglevel = RMGLog::summary;

  auto log_level_strings = magic_enum::enum_names<RMGLog::LogLevel>();
  auto log_level_desc = fmt::format("Logging level {}", fmt::join(log_level_strings, "|")).c_str();

  app.add_flag("-q", quiet, "Print only warnings and errors");
  app.add_flag("-v", verbosity, "Increase verbosity");
  app.add_option("-l,--log-level", loglevel, log_level_desc)->type_name("LEVEL")->default_val("summary");

  app.add_flag("-i,--interactive", interactive, "Run in interactive mode");
  app.add_option("-t,--threads", nthreads, "Number of threads");
  app.add_option("-g,--gdml-files", gdmls, "GDML files")->type_name("FILE");
  app.add_option("-o,--output-file", output, "Output file for detector hits")->type_name("FILE");
  app.add_option("macros", macros, "Macro files")->type_name("FILE");

  CLI11_PARSE(app, argc, argv);

  RMGLog::SetLogLevel(loglevel);

  switch (verbosity) {
    case 1: RMGLog::SetLogLevel(RMGLog::detail); break;
    case 2: RMGLog::SetLogLevel(RMGLog::debug); break;
    default: break;
  }

  if (quiet) RMGLog::SetLogLevel(RMGLog::warning);

  RMGManager manager("remage", argc, argv);
  manager.SetInteractive(interactive);
  manager.SetNumberOfThreads(nthreads);

  for (const auto& g : gdmls) manager.GetDetectorConstruction()->IncludeGDMLFile(g);
  for (const auto& m : macros) manager.IncludeMacroFile(m);
  if (!output.empty()) manager.SetOutputFileName(output);

  manager.Initialize();
  manager.Run();

  return 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include <csignal>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "G4Version.hh"

#include "RMGConfig.hh"
#include "RMGHardware.hh"
#include "RMGIpc.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGTools.hh"
#include "RMGVersion.hh"

#include "magic_enum/magic_enum.hpp"

namespace CLI::detail {
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
} // namespace CLI::detail

#include "CLI/CLI.hpp"

void signal_handler(int) { RMGManager::AbortRunGracefully(); }

int main(int argc, char** argv) {

  CLI::App app{"remage: simulation framework for germanium experiments"};

  int verbose = false;
  bool quiet = false;
  bool version = false;
  bool version_rich = false;
  bool no_banner = false;
  int nthreads = 1;
  int rand_seed = -1;
  bool interactive = false;
  bool overwrite_output = false;
  int pipe_fd_out = -1, pipe_fd_in = -1;
  int proc_num_offset = -1;
  std::vector<std::string> gdmls;
  std::vector<std::string> macros;
  std::vector<std::string> macro_substitutions;
  std::string output;
  RMGLog::LogLevel loglevel = RMGLog::summary;

  auto log_level_desc = "Logging level " + RMGTools::GetCandidates<RMGLog::LogLevel>('|');

  app.add_flag("-q,--quiet", quiet, "Print only warnings and errors (same as --log-level=warning)");
  app.add_flag(
      "-v,--verbose",
      verbose,
      "Increase program verbosity to maximum (same as --log-level=debug)"
  );
  app.add_flag("--version", version, "Print remage's version and exit");
  app.add_flag("--version-rich", version_rich, "Print versions of remage and its dependencies and exit");
  app.add_option("-l,--log-level", loglevel, log_level_desc)->type_name("LEVEL")->default_val("summary");
  app.add_flag("--no-banner", no_banner, "Do not print the remage banner at application start");

  app.add_option(
      "-s,--macro-substitutions",
      macro_substitutions,
      "key=value pairs of variables to substitute in macros (syntax as for Geant4 aliases)"
  );
  app.add_flag("-i,--interactive", interactive, "Open an interactive macro command prompt");
  app.add_option("-t,--threads", nthreads, "Set the number of threads used by remage");
  app.add_option("--rand-seed", rand_seed, "Set the random engine seed")->type_name("INT");
  app.add_option(
         "-g,--gdml-files",
         gdmls,
         "Supply one or more GDML files describing the experimental geometry"
  )
      ->type_name("FILE");
  app.add_option("-o,--output-file", output, "Output file for detector hits")->type_name("FILE");
  app.add_flag("-w,--overwrite", overwrite_output, "Overwrite existing output files");
  app.add_option(
         "--pipe-o-fd",
         pipe_fd_out,
         "Pipe file descriptor for inter-process communication, output (internal)"
  )
      ->group(""); // group("") hides the option from help output.
  app.add_option(
         "--pipe-i-fd",
         pipe_fd_in,
         "Pipe file descriptor for inter-process communication, input (internal)"
  )
      ->group(""); // group("") hides the option from help output.
  app.add_option(
         "--proc-num-offset",
         proc_num_offset,
         "process number for offset calculations in pseudo-multithreading mode (internal)"
  )
      ->group(""); // group("") hides the option from help output.
  app.add_option(
      "command_listings",
      macros,
      "One or more remage/Geant4 macro command listings to execute"
  );

  CLI11_PARSE(app, argc, argv);

  RMGLog::SetLogLevel(loglevel);

  if (version) {
    std::cout << RMG_PROJECT_VERSION << std::endl;
    return 0;
  }

  if (no_banner) RMGLog::SetInihibitStartupInfo(true);

  if (version_rich) {
    auto g4_version = std::regex_replace(G4Version, std::regex("\\$|Name:"), "");
    RMGLog::StartupInfo();
    std::cout << "version: " << RMG_PROJECT_VERSION << "\n\n"
              << "· Geant4 version: " << g4_version << "\n"
              << "· ROOT CERN support: " << (RMG_HAS_ROOT ? "yes" : "no") << "\n"
              << "· BxDecay0 support: "
              << (RMG_HAS_BXDECAY0 or RMG_HAS_BXDECAY0_THREADSAFE ? "yes" : "no") << "\n"
              << "· GDML support: " << (RMG_HAS_GDML ? "yes" : "no") << "\n"
              << "· HDF5 support: " << (RMG_HAS_HDF5 ? "yes" : "no") << std::endl;
    return 0;
  }

  if (verbose) RMGLog::SetLogLevel(RMGLog::debug);
  if (quiet) RMGLog::SetLogLevel(RMGLog::warning);

  // handle signal to abort the current run.
  struct sigaction sig{};
  sig.sa_handler = &signal_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  if (sigaction(SIGUSR1, &sig, nullptr) != 0) {
    RMGLog::Out(RMGLog::error, "signal install failed.");
  }

  RMGIpc::Setup(pipe_fd_out, pipe_fd_in, proc_num_offset > 0 ? proc_num_offset : 0);
  if (proc_num_offset >= 0) RMGLog::SetProcNum(proc_num_offset);
  // send general-purpose information to the python wrapper.
  RMGIpc::SendIpcNonBlocking(RMGIpc::CreateMessage("overwrite_output", overwrite_output ? "1" : "0"));

  RMGManager manager("remage", argc, argv);
  manager.SetInteractive(interactive);
  manager.GetOutputManager()->SetOutputOverwriteFiles(overwrite_output);
  if (nthreads > 1 && proc_num_offset >= 0) {
    RMGLog::Out(RMGLog::fatal, "invalid configuration for multi-threading or multi-processing.");
  }
  manager.SetNumberOfThreads(nthreads);
  if (proc_num_offset >= 0) manager.EnableMultiProcessing(proc_num_offset);
  if (rand_seed >= 0) manager.SetRandEngineSeed(rand_seed);

  for (const auto& g : gdmls) manager.GetDetectorConstruction()->IncludeGDMLFile(g);

  for (const auto& s : macro_substitutions) {
    size_t pos = s.find('=');
    if (pos != std::string::npos) {
      manager.RegisterG4Alias(s.substr(0, pos), s.substr(pos + 1));
    } else {
      RMGLog::Out(RMGLog::error, "invalid substitution pair ", s);
    }
  }

  for (const auto& m : macros) manager.IncludeMacroFile(m);
  if (!output.empty()) manager.GetOutputManager()->SetOutputFileName(output);

  manager.Initialize();
  manager.Run();

  if (manager.HadError()) return 1;
  return manager.HadWarning() ? 2 : 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

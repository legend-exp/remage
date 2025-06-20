// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#include <filesystem>
#include <string>
#include <sys/resource.h>
#include <vector>
namespace fs = std::filesystem;

#include "RMGConvertLH5.hh"
#include "RMGLog.hh"

#include "CLI/CLI.hpp"

int main(int argc, char** argv) {
  bool verbosity = false;
  bool dry_run = false;
  std::vector<std::string> file_names;
  std::set<std::string> aux_ntuples = {}; // not implemented
  std::string ntuple_group_name = "stp";

  CLI::App app{"remage-from-lh5: convert HDF5 file output files in-place to LH5"};
  app.add_flag("-v", verbosity, "Increase verbosity");
  app.add_flag("-n,--dry-run", dry_run, "Do not modify the on-disk files, only test the changes (on a full in-memory copy of the file)");
  app.add_option("--ntuple-group", ntuple_group_name, "HDF5 group name that remage was instructed to use")
      ->capture_default_str();
  app.add_option(
         "--aux-ntuples",
         aux_ntuples,
         "List of auxiliary ntuples to be pulled out of the main HDF5 group"
  )
      ->expected(0, -1)
      ->capture_default_str();
  app.add_option("input_files", file_names, "Input HDF5 files")->type_name("FILE")->required();
  CLI11_PARSE(app, argc, argv);

  RMGLog::SetInihibitStartupInfo(true);
  if (verbosity) RMGLog::SetLogLevel(RMGLog::detail);

  if (aux_ntuples.size() > 0) {
    RMGLog::Out(RMGLog::fatal, "Handling of auxiliary ntuples is not implemented yet");
  }

  RMGConvertLH5::fIsStandalone = true;

  for (auto& file_name : file_names) {
    if (!fs::exists(file_name)) {
      RMGLog::OutFormat(RMGLog::error, "{} does not exist", file_name);
      continue;
    }
    std::map<std::string, std::map<std::string, std::string>> units_map{};
    RMGConvertLH5::ConvertFromLH5(
        file_name,
        ntuple_group_name,
        aux_ntuples,
        dry_run,
        !file_names.empty(),
        units_map
    );
  }

  struct rusage usage{};
  if (!getrusage(RUSAGE_SELF, &usage)) {
    RMGLog::OutFormat(
        RMGLog::debug,
        "peak memory usage: {} MiB",
        usage.ru_maxrss / 1024
    ); // maxrss is in kilobytes.
  }

  return RMGLog::HadError() ? 1 : 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

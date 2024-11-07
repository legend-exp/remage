// Copyright (C) 2024 Manuel Huber
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

#include "CLI11/CLI11.hpp"

int main(int argc, char** argv) {
  bool verbosity;
  bool dry_run;
  std::vector<std::string> file_names;
  std::string ntuple_group_name = "stp";

  CLI::App app{"remage-to-lh5: convert HDF5 file output files in-place to LH5"};
  app.add_flag("-v", verbosity, "Increase verbosity");
  app.add_flag("-n,--dry-run",
      dry_run, "Do not modify the on-disk files, only test the changes (on a full in-memory copy of the file)");
  app.add_option("--ntuple-group", ntuple_group_name,
         "HDF5 group name that remage was instructed to use")
      ->capture_default_str();
  app.add_option("input_files", file_names, "Input HDF5 files")->type_name("FILE")->required();
  CLI11_PARSE(app, argc, argv);

  RMGLog::SetInihibitStartupInfo(true);
  if (verbosity) RMGLog::SetLogLevel(RMGLog::detail);

  RMGConvertLH5::fIsStandalone = true;

  for (auto& file_name : file_names) {
    if (!fs::exists(file_name)) {
      RMGLog::OutFormat(RMGLog::error, "{} does not exist", file_name);
      continue;
    }
    RMGConvertLH5::ConvertToLH5(file_name, ntuple_group_name, dry_run, !file_names.empty());
  }

  struct rusage usage{};
  if (!getrusage(RUSAGE_SELF, &usage)) {
    RMGLog::OutFormat(RMGLog::debug, "peak memory usage: {} MiB",
        usage.ru_maxrss / 1024); // maxrss is in kilobytes.
  }

  return RMGLog::HadError() ? 1 : 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include <filesystem>
#include <random>

#include "RMGVertexFromFile.hh"
namespace fs = std::filesystem;

#include "G4AnalysisUtilities.hh"
#include "G4RootAnalysisReader.hh"
#include "G4Threading.hh"
#include "G4VAnalysisReader.hh"
#if RMG_HAS_HDF5
#include "G4Hdf5AnalysisReader.hh"
#endif
#include "G4CsvAnalysisReader.hh"
#include "G4XmlAnalysisReader.hh"

#if RMG_HAS_HDF5
#include "RMGConvertLH5.hh"
#endif

#include "RMGLog.hh"

bool RMGAnalysisReader::OpenFile(std::string& name, std::string ntuple_dir_name,
    std::string ntuple_name) {

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) return false;

  if (fReader) return false;

  fFileIsTemp = false;

  if (!fs::exists(fs::path(name))) {
    RMGLog::Out(RMGLog::error, "input file ", name, " does not exist");
    return false;
  }

  // NOTE: GetExtension() returns a default extension if there is no file extension
  auto ext = G4Analysis::GetExtension(name);
  if (ext == "root") {
    fReader = G4RootAnalysisReader::Instance();
  } else if (ext == "hdf5") {
#if RMG_HAS_HDF5
    fReader = G4Hdf5AnalysisReader::Instance();
#else
    RMGLog::Out(RMGLog::fatal,
        "HDF5 input not available, please recompile Geant4 with HDF5 support");
#endif
  } else if (ext == "lh5") {
#if RMG_HAS_HDF5
    std::uniform_int_distribution<int> dist(10000, 99999);
    std::random_device rd;
    auto path = fs::path(name);
    std::string new_fn =
        ".rmg-vtx-" + std::to_string(dist(rd)) + "." + path.stem().string() + ".hdf5";

    std::error_code ec;
    if (!fs::copy_file(path, fs::path(new_fn)), ec) {
      RMGLog::Out(RMGLog::error, "copy of input file ", name, " failed. Does it exist? (",
          ec.message(), ")");
      return false;
    }

    if (!RMGConvertLH5::ConvertFromLH5(new_fn, ntuple_dir_name, false)) {
      RMGLog::Out(RMGLog::error, "Conversion of input file ", new_fn,
          " to LH5 failed. Data is potentially corrupted.");
      return false;
    }

    name = new_fn;
    fReader = G4Hdf5AnalysisReader::Instance();
    fFileIsTemp = true;
#else
    RMGLog::Out(RMGLog::fatal,
        "HDF5 input not available, please recompile Geant4 with HDF5 support");
#endif
  } else if (ext == "csv") {
    fReader = G4CsvAnalysisReader::Instance();
  } else if (ext == "xml") {
    fReader = G4XmlAnalysisReader::Instance();
  } else {
    RMGLog::OutFormat(RMGLog::fatal, "File Extension '.{}' not recognized!");
  }

  if (RMGLog::GetLogLevel() <= RMGLog::debug) fReader->SetVerboseLevel(10);

  fReader->SetFileName(name);
  fFileName = name;

  fNtupleId = fReader->GetNtuple(ntuple_name);
  if (fNtupleId < 0) {
    RMGLog::Out(RMGLog::error, "Ntuple named '", ntuple_name, "' could not be found in input file!");
    return false;
  }
  return true;
}

void RMGAnalysisReader::CloseFile() {
  if (!G4Threading::IsMasterThread()) return;

  if (!fReader) return;

  fReader = nullptr;
  if (fFileIsTemp) {
    std::error_code ec;
    fs::remove(fs::path(fFileName), ec);
  }
  fFileName = "";
  fFileIsTemp = false;
}

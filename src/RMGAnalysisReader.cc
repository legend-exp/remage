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

#include "RMGAnalysisReader.hh"

#include <filesystem>
#include <random>
namespace fs = std::filesystem;

#include "G4AnalysisUtilities.hh"
#include "G4CsvAnalysisReader.hh"
#if RMG_HAS_HDF5
#include "G4Hdf5AnalysisReader.hh"
#endif
#include "G4RootAnalysisReader.hh"
#include "G4Threading.hh"
#include "G4XmlAnalysisReader.hh"

#if RMG_HAS_HDF5
#include "RMGConvertLH5.hh"
#endif
#include "RMGIpc.hh"
#include "RMGLog.hh"

G4Mutex RMGAnalysisReader::fMutex = G4MUTEX_INITIALIZER;

RMGAnalysisReader::Access RMGAnalysisReader::OpenFile(
    const std::string& file_name,
    std::string ntuple_dir_name,
    std::string ntuple_name,
    std::string force_ext
) {

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
    std::abort();
  }

  G4AutoLock lock(&fMutex);

  return OpenFile(file_name, ntuple_dir_name, ntuple_name, std::move(lock), force_ext);
}

RMGAnalysisReader::Access RMGAnalysisReader::OpenFile(
    const std::string& file_name,
    std::string ntuple_dir_name,
    std::string ntuple_name,
    G4AutoLock lock,
    std::string force_ext
) {

  if (lock.mutex() != &fMutex || !lock) {
    RMGLog::OutDev(RMGLog::fatal, "used lock protecting the wrong mutex or unheld lock");
    std::abort();
  }

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
    std::abort();
  }

  // create an invalid lock to return on error.
  G4AutoLock invalid_lock(&fMutex, std::defer_lock);
  invalid_lock.release();
  auto invalid_access = RMGAnalysisReader::Access(std::move(invalid_lock), nullptr, -1, nullptr, false);

  if (fReader) return invalid_access;

  fFileIsTemp = false;
  fFileName = file_name;
  auto path = fs::path(file_name);

  if (!fs::exists(path)) {
    RMGLog::Out(RMGLog::error, "input file ", file_name, " does not exist");
    return invalid_access;
  }

  std::map<std::string, std::map<std::string, std::string>> units_map{};

  auto ext = !force_ext.empty() ? force_ext : G4Analysis::GetExtension(file_name);
  if (ext == "root") {
    fReader = G4RootAnalysisReader::Instance();
  } else if (ext == "hdf5") {
#if RMG_HAS_HDF5
    fReader = G4Hdf5AnalysisReader::Instance();
#else
    RMGLog::Out(RMGLog::fatal, "HDF5 input not available, please recompile Geant4 with HDF5 support");
#endif
  } else if (ext == "lh5") {
#if RMG_HAS_HDF5
    std::uniform_int_distribution<int> dist(10000, 99999);
    std::random_device rd;
    std::string new_fn = ".rmg-vtx-" + std::to_string(dist(rd)) + "." + path.stem().string() +
                         ".hdf5";

    std::error_code ec;
    if (!fs::copy_file(path, fs::path(new_fn), ec)) {
      RMGLog::Out(
          RMGLog::error,
          "copy of input file ",
          file_name,
          " failed. Does it exist? (",
          ec.message(),
          ")"
      );
      return invalid_access;
    }
    RMGIpc::SendIpcNonBlocking(RMGIpc::CreateMessage("tmpfile", new_fn));

    if (!RMGConvertLH5::ConvertFromLH5(new_fn, ntuple_dir_name, false, false, units_map)) {
      RMGLog::Out(
          RMGLog::error,
          "Conversion of input file ",
          new_fn,
          " to LH5 failed. Data is potentially corrupted."
      );
      return invalid_access;
    }

    fHasUnits = true;
    fFileName = new_fn;
    fReader = G4Hdf5AnalysisReader::Instance();
    fFileIsTemp = true;
#else
    RMGLog::Out(RMGLog::fatal, "HDF5 input not available, please recompile Geant4 with HDF5 support");
#endif
  } else if (ext == "csv") {
    fReader = G4CsvAnalysisReader::Instance();
  } else if (ext == "xml") {
    fReader = G4XmlAnalysisReader::Instance();
  } else {
    RMGLog::OutFormat(RMGLog::fatal, "File Extension '.{}' not recognized!", ext);
  }

  if (RMGLog::GetLogLevel() <= RMGLog::debug) fReader->SetVerboseLevel(10);

  fNtupleId = fReader->GetNtuple(ntuple_name, fFileName, ntuple_dir_name);
  if (fNtupleId < 0) {
    RMGLog::Out(RMGLog::error, "Ntuple named '", ntuple_name, "' could not be found in input file!");
    return invalid_access;
  }
  fUnits = units_map[ntuple_name];
  return {std::move(lock), fReader, fNtupleId, fHasUnits ? &fUnits : nullptr, true};
}

void RMGAnalysisReader::CloseFile() {
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
    return;
  }

  G4AutoLock lock(&fMutex);

  if (!fReader) return;

  // fReader is a thread-local singleton. Do not delete it here, otherwise geant4 will to delete it
  // again. also do not close files, as there is no way to close a specific file only.
  fReader = nullptr;
  if (fFileIsTemp) {
    std::error_code ec;
    fs::remove(fs::path(fFileName), ec);
  }
  fFileName = "";
  fFileIsTemp = false;
  fNtupleId = -1;
  fHasUnits = false;
  fHasUnits = {};
}

RMGAnalysisReader::Access RMGAnalysisReader::GetLockedReader() const {

  G4AutoLock lock(&fMutex);

  return {std::move(lock), fReader, fNtupleId, fHasUnits ? &fUnits : nullptr, false};
}

G4AutoLock RMGAnalysisReader::GetLock() const {

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
    std::abort();
  }

  G4AutoLock lock(&fMutex);

  return lock;
}

/* ========================================================================================== */

std::string RMGAnalysisReader::Access::GetUnit(const std::string& name) const {
  if (!fUnits) return "";
  return fUnits->at(name);
}

void RMGAnalysisReader::Access::AssertUnit(
    const std::string& name,
    const std::vector<std::string>& allowed_units
) const {
  if (!fUnits) return;
  if (std::find(allowed_units.begin(), allowed_units.end(), GetUnit(name)) == allowed_units.end()) {
    RMGLog::Out(RMGLog::fatal, "invalid unit '", GetUnit(name), "' for column ", name);
  }
}

void RMGAnalysisReader::Access::AssertSetup(bool setup) const {
  if (setup != fCanSetup) {
    RMGLog::Out(RMGLog::fatal, "invalid operation in mode fCanSetup=", fCanSetup);
  }
}

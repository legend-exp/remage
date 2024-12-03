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

#include "RMGVertexFromFile.hh"

#include <filesystem>
#include <random>
namespace fs = std::filesystem;

#include "CLHEP/Units/SystemOfUnits.h"
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

namespace {
  G4Mutex RMGVertexFromFileMutex = G4MUTEX_INITIALIZER;
} // namespace

G4VAnalysisReader* RMGVertexFromFile::fReader = nullptr;

RMGVertexFromFile::RMGVertexFromFile() : RMGVVertexGenerator("FromFile") { this->DefineCommands(); }

void RMGVertexFromFile::OpenFile(std::string& name) {

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) return;

  G4AutoLock lock(&RMGVertexFromFileMutex);
  if (fReader) return;

  fFileIsTemp = false;

  if (!fs::exists(fs::path(name))) {
    RMGLog::Out(RMGLog::error, "input file ", name, " does not exist");
    return;
  }

  // NOTE: GetExtension() returns a default extension if there is no file extension
  auto ext = G4Analysis::GetExtension(name);
  if (ext == "root") fReader = G4RootAnalysisReader::Instance();
  else if (ext == "hdf5") {
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
      return;
    }

    if (!RMGConvertLH5::ConvertFromLH5(new_fn, fNtupleDirectoryName, false)) {
      RMGLog::Out(RMGLog::error, "Conversion of input file ", new_fn,
          " to LH5 failed. Data is potentially corrupted.");
      return;
    }

    name = new_fn;
    fReader = G4Hdf5AnalysisReader::Instance();
    fFileIsTemp = true;
#else
    RMGLog::Out(RMGLog::fatal,
        "HDF5 input not available, please recompile Geant4 with HDF5 support");
#endif
  } else if (ext == "csv") fReader = G4CsvAnalysisReader::Instance();
  else if (ext == "xml") fReader = G4XmlAnalysisReader::Instance();
  else { RMGLog::OutFormat(RMGLog::fatal, "File Extension '.{}' not recognized!"); }

  if (RMGLog::GetLogLevel() <= RMGLog::debug) fReader->SetVerboseLevel(10);

  fReader->SetFileName(name);
  fFileName = name;

  fNtupleId = fReader->GetNtuple("pos");
  if (fNtupleId >= 0) {
    // bind the static variables once here, and only use them later.
    fReader->SetNtupleDColumn("xloc_in_m", fXpos);
    fReader->SetNtupleDColumn("yloc_in_m", fYpos);
    fReader->SetNtupleDColumn("zloc_in_m", fZpos);
  } else {
    RMGLog::Out(RMGLog::error, "Ntuple named 'pos' could not be found in input file!");
  }
}

bool RMGVertexFromFile::GenerateVertex(G4ThreeVector& vertex) {

  G4AutoLock lock(&RMGVertexFromFileMutex);

  if (fReader && fNtupleId >= 0) {
    fXpos = fYpos = fZpos = NAN;

    if (fReader->GetNtupleRow()) {
      // check for NaN sentinel values - i.e. non-existing columns (there is no error message).
      if (std::isnan(fXpos) || std::isnan(fYpos) || std::isnan(fZpos)) {
        RMGLog::Out(RMGLog::error, "At least one of the columns does not exist");
        vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
        return false;
      }

      vertex = G4ThreeVector{fXpos, fYpos, fZpos} * CLHEP::m;
      return true;
    }

    RMGLog::Out(RMGLog::error, "No more vertices available in input file!");
  } else {
    RMGLog::Out(RMGLog::error, "Ntuple named 'pos' could not be found in input file!");
  }

  vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
  return false;
}

void RMGVertexFromFile::BeginOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;

  G4AutoLock lock(&RMGVertexFromFileMutex);
  if (!fReader) {
    RMGLog::Out(RMGLog::fatal, "vertex file '", fFileName, "' not found or in wrong format");
  }
}

void RMGVertexFromFile::EndOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;

  G4AutoLock lock(&RMGVertexFromFileMutex);
  if (!fReader) return;

  fReader = nullptr;
  if (fFileIsTemp) {
    std::error_code ec;
    fs::remove(fs::path(fFileName), ec);
  }
  fFileName = "";
  fFileIsTemp = false;
}

void RMGVertexFromFile::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/FromFile/",
      "Commands for controlling reading event vertex positions from file");

  fMessenger->DeclareMethod("FileName", &RMGVertexFromFile::OpenFile)
      .SetGuidance("Set name of the file containing vertex positions for the next run. See the "
                   "documentation for a specification of the format.")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareProperty("NtupleDirectory", fNtupleDirectoryName)
      .SetGuidance("Change the default input directory/group for ntuples.")
      .SetGuidance("note: this option only has an effect for LH5 input files.")
      .SetParameterName("nt_directory", false)
      .SetDefaultValue(fNtupleDirectoryName)
      .SetStates(G4State_PreInit, G4State_Idle);
}

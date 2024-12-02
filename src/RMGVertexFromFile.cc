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

#include "G4AnalysisUtilities.hh"
#include "G4RootAnalysisReader.hh"
#include "G4Threading.hh"
#include "G4VAnalysisReader.hh"
#ifdef TOOLS_USE_HDF5
#include "G4Hdf5AnalysisReader.hh"
#endif
#include "G4CsvAnalysisReader.hh"
#include "G4XmlAnalysisReader.hh"

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

  // NOTE: GetExtension() returns a default extension if there is no file extension
  auto ext = G4Analysis::GetExtension(name);
  if (ext == "root") fReader = G4RootAnalysisReader::Instance();
  else if (ext == "hdf5") {
#ifdef TOOLS_USE_HDF5
    fReader = G4Hdf5AnalysisReader::Instance();
#else
    RMGLog::Out(RMGLog::fatal,
        "HDF5 input not available, please recompile Geant4 with HDF5 support");
#endif
  } else if (ext == "csv") fReader = G4CsvAnalysisReader::Instance();
  else if (ext == "xml") fReader = G4XmlAnalysisReader::Instance();
  else { RMGLog::OutFormat(RMGLog::fatal, "File Extension '.{}' not recognized!"); }

  if (RMGLog::GetLogLevel() <= RMGLog::debug) fReader->SetVerboseLevel(10);

  fReader->SetFileName(name);

  fNtupleId = fReader->GetNtuple("vertices");
  if (fNtupleId >= 0) {
    // bind the static variables once here, and only use them later.
    fReader->SetNtupleDColumn("xloc_in_m", fXpos);
    fReader->SetNtupleDColumn("yloc_in_m", fYpos);
    fReader->SetNtupleDColumn("zloc_in_m", fZpos);
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

      vertex = G4ThreeVector{fXpos, fYpos, fZpos};
      return true;
    }

    RMGLog::Out(RMGLog::error, "No more vertices available in input file!");
  } else {
    RMGLog::Out(RMGLog::error, "Ntuple named 'vertices' could not be found in input file!");
  }

  vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
  return false;
}

void RMGVertexFromFile::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Confinement/FromFile/",
      "Commands for controlling reading event vertex positions from file");

  fMessenger->DeclareMethod("FileName", &RMGVertexFromFile::OpenFile)
      .SetGuidance("Set name of the file containing vertex positions. See the documentation for a "
                   "specification of the format.")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);
}

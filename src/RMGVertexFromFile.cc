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
#include "G4VAnalysisReader.hh"
#ifdef TOOLS_USE_HDF5
#include "G4Hdf5AnalysisReader.hh"
#endif
#include "G4CsvAnalysisReader.hh"
#include "G4XmlAnalysisReader.hh"

#include "RMGLog.hh"

RMGVertexFromFile::RMGVertexFromFile() : RMGVVertexGenerator("FromFile") { this->DefineCommands(); }

void RMGVertexFromFile::OpenFile(std::string& name) {
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
}

bool RMGVertexFromFile::GeneratePrimariesVertex(G4ThreeVector& vertex) {

  auto ntupleid = fReader->GetNtuple("vertices");
  if (ntupleid >= 0) {
    double xpos, ypos, zpos;
    fReader->SetNtupleDColumn("xpos", xpos);
    fReader->SetNtupleDColumn("ypos", ypos);
    fReader->SetNtupleDColumn("zpos", zpos);

    if (fReader->GetNtupleRow()) {
      vertex = G4ThreeVector{xpos, ypos, zpos};
      return true;
    } else RMGLog::Out(RMGLog::error, "No more vertices available in input file!");
    vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
    return false;
  } else {
    RMGLog::Out(RMGLog::fatal, "Ntuple named 'vertices' could not be found in input file!");
    return false;
  }
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

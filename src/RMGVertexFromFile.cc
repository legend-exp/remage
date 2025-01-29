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

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4Threading.hh"

#include "RMGLog.hh"

RMGAnalysisReader* RMGVertexFromFile::fReader = new RMGAnalysisReader();

RMGVertexFromFile::RMGVertexFromFile() : RMGVVertexGenerator("FromFile") { this->DefineCommands(); }

void RMGVertexFromFile::OpenFile(std::string& name) {

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) return;

  auto reader = fReader->OpenFile(name, fNtupleDirectoryName, "pos");
  if (!reader) return;

  // bind the static variables once here, and only use them later.
  reader.SetNtupleDColumn("xloc", fXpos, {"nm", "um", "mm", "cm", "m"});
  reader.SetNtupleDColumn("yloc", fYpos, {"nm", "um", "mm", "cm", "m"});
  reader.SetNtupleDColumn("zloc", fZpos, {"nm", "um", "mm", "cm", "m"});

  const auto xunit = reader.GetUnit("xloc");
  const auto yunit = reader.GetUnit("yloc");
  const auto zunit = reader.GetUnit("zloc");
  if (xunit != yunit || xunit != zunit) {
    RMGLog::OutFormat(RMGLog::fatal, " ('{}', '{}', '{}')", xunit, yunit, zunit);
  }
}

bool RMGVertexFromFile::GenerateVertex(G4ThreeVector& vertex) {

  auto reader = fReader->GetLockedReader();

  if (reader) {
    fXpos = fYpos = fZpos = NAN; // initialize sentinel values.

    if (reader.GetNtupleRow()) {
      // check for NaN sentinel values - i.e. non-existing columns (there is no error message).
      if (std::isnan(fXpos) || std::isnan(fYpos) || std::isnan(fZpos)) {
        RMGLog::Out(RMGLog::error, "At least one of the columns does not exist");
        vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
        return false;
      }

      const auto unit_name = reader.GetUnit("xloc");
      const std::map<std::string, double> units = {{"", CLHEP::m}, {"nm", CLHEP::nm},
          {"um", CLHEP::um}, {"mm", CLHEP::mm}, {"cm", CLHEP::cm}, {"m", CLHEP::m}};

      vertex = G4ThreeVector{fXpos, fYpos, fZpos} * units.at(unit_name);
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

  if (!fReader->GetLockedReader()) {
    RMGLog::Out(RMGLog::fatal, "vertex file '", fReader->GetFileName(),
        "' not found or in wrong format");
  }
}

void RMGVertexFromFile::EndOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;

  fReader->CloseFile();
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
      .SetGuidance("note: this option only has an effect for LH5 or HDF5 input files.")
      .SetParameterName("nt_directory", false)
      .SetDefaultValue(fNtupleDirectoryName)
      .SetStates(G4State_PreInit, G4State_Idle);
}

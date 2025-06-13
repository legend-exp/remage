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

#include "RMGGeneratorFromFile.hh"

#include <cmath>

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"

#include "RMGLog.hh"

namespace u = CLHEP;

RMGAnalysisReader* RMGGeneratorFromFile::fReader = new RMGAnalysisReader();

RMGGeneratorFromFile::RMGGeneratorFromFile() : RMGVGenerator("FromFile") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

void RMGGeneratorFromFile::OpenFile(std::string& name) {

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) return;

  auto reader = fReader->OpenFile(name, fNtupleDirectoryName, "kin");
  if (!reader) return;

  // bind the static variables once here, and only use them later.
  reader.SetNtupleIColumn("g4_pid", fRowData.fG4Pid, {""});
  reader.SetNtupleDColumn("ekin", fRowData.fEkin, {"eV", "keV", "MeV", "GeV"});
  reader.SetNtupleDColumn("px", fRowData.fPx, {""});
  reader.SetNtupleDColumn("py", fRowData.fPy, {""});
  reader.SetNtupleDColumn("pz", fRowData.fPz, {""});
}

void RMGGeneratorFromFile::BeginOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;

  if (!fReader->GetLockedReader()) {
    RMGLog::Out(RMGLog::fatal, "vertex file '", fReader->GetFileName(), "' not found or in wrong format");
  }
}

void RMGGeneratorFromFile::EndOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;

  fReader->CloseFile();
}


void RMGGeneratorFromFile::GeneratePrimaries(G4Event* event) {

  auto locked_reader = fReader->GetLockedReader();

  if (!locked_reader) {
    RMGLog::Out(RMGLog::error, "Ntuple named 'pos' could not be found in input file!");
    return;
  }

  fRowData = RowData(); // initialize sentinel values.

  if (!locked_reader.GetNtupleRow()) {
    RMGLog::Out(RMGLog::error, "No more vertices available in input file!");
    return;
  }

  // make copy of data and exit critical section.
  auto row_data = fRowData;
  auto unit_name = locked_reader.GetUnit("ekin");
  locked_reader.unlock();

  // check for NaN sentinel values - i.e. non-existing columns (there is no error message).
  if (!row_data.IsValid()) {
    RMGLog::Out(RMGLog::error, "At least one of the columns does not exist");
    return;
  }

  auto particle = G4ParticleTable::GetParticleTable()->FindParticle(row_data.fG4Pid);
  if (!particle) {
    RMGLog::Out(RMGLog::error, "invalid particle PDG id ", row_data.fG4Pid);
    return;
  }

  RMGLog::OutFormat(
      RMGLog::debug,
      "particle {:d} (px,py,pz) = ({:.4g}, {:.4g}, {:.4g}); Ekin = {:.4g} MeV",
      row_data.fG4Pid,
      row_data.fPx,
      row_data.fPy,
      row_data.fPz,
      row_data.fEkin
  );

  G4ThreeVector momentum{row_data.fPx, row_data.fPy, row_data.fPz};

  const std::map<std::string, double> units =
      {{"", u::MeV}, {"eV", u::eV}, {"keV", u::keV}, {"MeV", u::MeV}, {"GeV", u::GeV}};

  fGun->SetParticleDefinition(particle);
  fGun->SetParticlePosition(fParticlePosition);
  fGun->SetParticleMomentumDirection(momentum);
  fGun->SetParticleEnergy(row_data.fEkin * units.at(unit_name));

  fGun->GeneratePrimaryVertex(event);
}

void RMGGeneratorFromFile::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Generator/FromFile/",
      "Commands for controlling reading event data from file"
  );

  fMessenger->DeclareMethod("FileName", &RMGGeneratorFromFile::OpenFile)
      .SetGuidance(
          "Set name of the file containing vertex kinetics for the next run. See the "
          "documentation for a specification of the format."
      )
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclareProperty("NtupleDirectory", fNtupleDirectoryName)
      .SetGuidance("Change the default input directory/group for ntuples.")
      .SetGuidance("note: this option only has an effect for LH5 or HDF5 input files.")
      .SetParameterName("nt_directory", false)
      .SetDefaultValue(fNtupleDirectoryName)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

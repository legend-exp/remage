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

#include "RMGGeneratorFromFile.hh"

#include <cmath>

#include "G4ParticleGun.hh"
#include "G4ParticleMomentum.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGTools.hh"
#include "RMGVGenerator.hh"

namespace u = CLHEP;

G4Mutex RMGGeneratorFromFile::fMutex = G4MUTEX_INITIALIZER;

RMGAnalysisReader* RMGGeneratorFromFile::fReader = new RMGAnalysisReader();

RMGGeneratorFromFile::RMGGeneratorFromFile() : RMGVGenerator("FromFile") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

void RMGGeneratorFromFile::OpenFile(std::string& name) {

  // reader initialization should only happen on the master thread (otherwise it will fail).
  if (!G4Threading::IsMasterThread()) return;
  G4AutoLock lock(&fMutex);

  if (!fReader->OpenFile(name, fNtupleDirectoryName, "kin")) return;

  auto nt = fReader->GetNtupleId();
  if (nt >= 0) {
    // bind the static variables once here, and only use them later.
    fReader->GetReader()->SetNtupleIColumn(nt, "g4_pid", fRowData.fG4Pid);
    fReader->GetReader()->SetNtupleDColumn(nt, "ekin_in_MeV", fRowData.fEkin);
    fReader->GetReader()->SetNtupleDColumn(nt, "px_in_MeV", fRowData.fPx);
    fReader->GetReader()->SetNtupleDColumn(nt, "py_in_MeV", fRowData.fPy);
    fReader->GetReader()->SetNtupleDColumn(nt, "pz_in_MeV", fRowData.fPz);
  }
}

void RMGGeneratorFromFile::BeginOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;
  G4AutoLock lock(&fMutex);

  if (!fReader->GetReader()) {
    RMGLog::Out(RMGLog::fatal, "vertex file '", fReader->GetFileName(),
        "' not found or in wrong format");
  }
}

void RMGGeneratorFromFile::EndOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;
  G4AutoLock lock(&fMutex);

  fReader->CloseFile();
}


void RMGGeneratorFromFile::GeneratePrimaries(G4Event* event) {

  G4AutoLock lock(&fMutex);

  if (!fReader->GetReader() || fReader->GetNtupleId() < 0) {
    RMGLog::Out(RMGLog::error, "Ntuple named 'pos' could not be found in input file!");
    return;
  }

  fRowData = RowData(); // initialize sentinel values.

  auto nt = fReader->GetNtupleId();
  if (!fReader->GetReader()->GetNtupleRow(nt)) {
    RMGLog::Out(RMGLog::error, "No more vertices available in input file!");
    return;
  }

  // check for NaN sentinel values - i.e. non-existing columns (there is no error message).
  if (!fRowData.IsValid()) {
    RMGLog::Out(RMGLog::error, "At least one of the columns does not exist");
    return;
  }

  auto particle = G4ParticleTable::GetParticleTable()->FindParticle(fRowData.fG4Pid);
  if (!particle) {
    RMGLog::Out(RMGLog::error, "invalid particle PDG id ", fRowData.fG4Pid);
    return;
  }

  RMGLog::OutFormat(RMGLog::debug,
      "particle {:d} (px,py,pz) = ({:.4g}, {:.4g}, {:.4g}) MeV ; Ekin = {:.4g} MeV",
      fRowData.fG4Pid, fRowData.fPx, fRowData.fPy, fRowData.fPz, fRowData.fEkin);

  G4ThreeVector momentum{fRowData.fPx, fRowData.fPy, fRowData.fPz};

  fGun->SetParticleDefinition(particle);
  fGun->SetParticlePosition(fParticlePosition);
  fGun->SetParticleMomentumDirection(momentum * u::MeV);
  fGun->SetParticleEnergy(fRowData.fEkin * u::MeV);

  fGun->GeneratePrimaryVertex(event);
}

void RMGGeneratorFromFile::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/FromFile/",
      "Commands for controlling reading event data from file");

  fMessenger->DeclareMethod("FileName", &RMGGeneratorFromFile::OpenFile)
      .SetGuidance("Set name of the file containing vertex kinetics for the next run. See the "
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

// vim: tabstop=2 shiftwidth=2 expandtab

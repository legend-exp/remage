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
#include <vector>

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4RunManager.hh"
#include "G4ThreeVector.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"

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
  reader.SetNtupleDColumn("time", fRowData.fTime, {"ns", "ms", "s"});
  reader.SetNtupleIColumn("n_part", fRowData.fNpart, {""});
}

void RMGGeneratorFromFile::BeginOfRunAction(const G4Run*) {

  if (!G4Threading::IsMasterThread()) return;

  auto reader = fReader->GetLockedReader();
  if (!reader) {
    RMGLog::Out(RMGLog::fatal, "vertex file '", fReader->GetFileName(), "' not found or in wrong format");
  }

  // in the multiprocessing-mode we get here with an offset on the main thread.
  size_t start_event = RMGManager::Instance()->GetProcessNumberOffset() *
                       G4RunManager::GetRunManager()->GetNumberOfEventsToBeProcessed();
  // skip the first start_event rows from the input file.
  if (start_event > 0) {
    size_t skipped_events = 0, skipped_rows = 0, skipped_rows_this_evt = 0, n_part_this_evt = 1;
    while ((skipped_events < start_event) ||
           (skipped_events == start_event && skipped_rows_this_evt < n_part_this_evt)) {
      fRowData = RowData(); // initialize sentinel values.

      if (!reader.GetNtupleRow()) {
        RMGLog::Out(RMGLog::fatal, "[initial seek] No more vertices available in input file!");
        break;
      }

      if (!fRowData.IsValid()) {
        RMGLog::Out(
            RMGLog::fatal,
            "[initial seek] At least one of the columns does not exist or of wrong type"
        );
        break;
      }

      if (fRowData.fNpart > 0) {
        skipped_events += 1;
        skipped_rows_this_evt = 0;
        n_part_this_evt = fRowData.fNpart;
      }

      skipped_rows_this_evt++;
      skipped_rows++;
    }
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

  std::vector<RowData> particles;

  int n_part = 1, n = 0;
  bool is_valid = true, is_valid_particle = true;
  while (n < n_part) {
    fRowData = RowData(); // initialize sentinel values.

    if (!locked_reader.GetNtupleRow()) {
      RMGLog::Out(RMGLog::error, "No more vertices available in input file!");
      return;
    }

    if (!fRowData.IsValid()) {
      is_valid = false;
      break;
    }

    particles.push_back(fRowData); // make copy of data.
    if (n == 0) {
      n_part = fRowData.fNpart;
      if (n_part <= 0) {
        is_valid_particle = false;
        break;
      }
    } else if (fRowData.fNpart != 0) {
      is_valid_particle = false;
      break;
    }
    n++;
  }

  // exit critical section.
  auto unit_ekin = locked_reader.GetUnit("ekin");
  auto unit_time = locked_reader.GetUnit("time");
  locked_reader.unlock();

  // check for NaN sentinel values - i.e. non-existing columns (there is no error message).
  if (!is_valid) {
    RMGLog::Out(RMGLog::error, "At least one of the columns does not exist or of wrong type");
    return;
  }
  if (!is_valid_particle || particles.empty()) {
    RMGLog::Out(RMGLog::error, "Event particle count not valid in input file");
    return;
  }

  const std::map<std::string, double> energy_units =
      {{"", u::MeV}, {"eV", u::eV}, {"keV", u::keV}, {"MeV", u::MeV}, {"GeV", u::GeV}};
  const std::map<std::string, double> time_units =
      {{"", u::ns}, {"ns", u::ns}, {"ms", u::ms}, {"s", u::s}};

  for (const auto& row_data : particles) {

    auto particle = G4ParticleTable::GetParticleTable()->FindParticle(row_data.fG4Pid);
    if (!particle) {
      RMGLog::Out(RMGLog::error, "invalid particle PDG id ", row_data.fG4Pid);
      return;
    }

    RMGLog::OutFormat(
        RMGLog::debug_event,
        "particle {:d} (px,py,pz) = ({:.4g}, {:.4g}, {:.4g}); Ekin = {:.4g} MeV; time = {:.4g} ns",
        row_data.fG4Pid,
        row_data.fPx,
        row_data.fPy,
        row_data.fPz,
        row_data.fEkin * energy_units.at(unit_ekin) / u::MeV,
        row_data.fTime * time_units.at(unit_time) / u::ns
    );

    G4ThreeVector momentum{row_data.fPx, row_data.fPy, row_data.fPz};

    fGun->SetParticleDefinition(particle);
    fGun->SetParticlePosition(fParticlePosition);
    fGun->SetParticleTime(row_data.fTime * time_units.at(unit_time));
    fGun->SetParticleMomentumDirection(momentum);
    fGun->SetParticleEnergy(row_data.fEkin * energy_units.at(unit_ekin));

    fGun->GeneratePrimaryVertex(event);
  }
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

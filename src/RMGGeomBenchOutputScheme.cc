// Copyright (C) 2025 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
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

#include "RMGGeomBenchOutputScheme.hh"

#include "G4AnalysisManager.hh"

#include "RMGLog.hh"
#include "RMGOutputManager.hh"

RMGGeomBenchOutputScheme::RMGGeomBenchOutputScheme() = default;

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGGeomBenchOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGOutputManager::Instance();

  // Create auxiliary ntuple for XZ plane benchmark
  fNtupleIDs[0] = rmg_man->CreateAndRegisterAuxNtuple("benchmark_xz", "RMGGeomBenchOutputScheme", ana_man);
  ana_man->CreateNtupleDColumn(fNtupleIDs[0], "x");
  ana_man->CreateNtupleDColumn(fNtupleIDs[0], "z");
  ana_man->CreateNtupleDColumn(fNtupleIDs[0], "time");
  ana_man->FinishNtuple(fNtupleIDs[0]);

  // Create auxiliary ntuple for YZ plane benchmark
  fNtupleIDs[1] = rmg_man->CreateAndRegisterAuxNtuple("benchmark_yz", "RMGGeomBenchOutputScheme", ana_man);
  ana_man->CreateNtupleDColumn(fNtupleIDs[1], "y");
  ana_man->CreateNtupleDColumn(fNtupleIDs[1], "z");
  ana_man->CreateNtupleDColumn(fNtupleIDs[1], "time");
  ana_man->FinishNtuple(fNtupleIDs[1]);

  // Create auxiliary ntuple for XY plane benchmark
  fNtupleIDs[2] = rmg_man->CreateAndRegisterAuxNtuple("benchmark_xy", "RMGGeomBenchOutputScheme", ana_man);
  ana_man->CreateNtupleDColumn(fNtupleIDs[2], "x");
  ana_man->CreateNtupleDColumn(fNtupleIDs[2], "y");
  ana_man->CreateNtupleDColumn(fNtupleIDs[2], "time");
  ana_man->FinishNtuple(fNtupleIDs[2]);
}

void RMGGeomBenchOutputScheme::SavePixel(int plane_id, double x, double y, double z, double time) {

  if (plane_id < 0 || plane_id > 2) {
    RMGLog::Out(RMGLog::warning, "Invalid plane_id (", plane_id, ") in SavePixel(); skipping");
    return;
  }

  auto rmg_man = RMGOutputManager::Instance();
  if (!rmg_man->IsPersistencyEnabled()) return;

  const auto ana_man = G4AnalysisManager::Instance();
  if (!ana_man) {
    RMGLog::Out(RMGLog::warning, "Analysis manager not available in SavePixel(); skipping");
    return;
  }

  int ntuple_id = fNtupleIDs[plane_id];
  if (ntuple_id < 0) {
    RMGLog::Out(RMGLog::warning, "Ntuple ID not initialized for plane_id=", plane_id);
    return;
  }

  int col_id = 0;
  switch (plane_id) {
    case 0: // XZ plane
      ana_man->FillNtupleDColumn(ntuple_id, col_id++, x);
      ana_man->FillNtupleDColumn(ntuple_id, col_id++, z);
      break;
    case 1: // YZ plane
      ana_man->FillNtupleDColumn(ntuple_id, col_id++, y);
      ana_man->FillNtupleDColumn(ntuple_id, col_id++, z);
      break;
    case 2: // XY plane
      ana_man->FillNtupleDColumn(ntuple_id, col_id++, x);
      ana_man->FillNtupleDColumn(ntuple_id, col_id++, y);
      break;
  }

  ana_man->FillNtupleDColumn(ntuple_id, col_id++, time);
  ana_man->AddNtupleRow(ntuple_id);
}

// vim: tabstop=2 shiftwidth=2 expandtab

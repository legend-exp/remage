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

#include "RMGVertexOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

RMGVertexOutputScheme::RMGVertexOutputScheme() { this->DefineCommands(); }

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGVertexOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {
  if (fSkipPrimaryVertexOutput) return;

  auto vid = RMGManager::Instance()->RegisterNtuple("vertices",
      ana_man->CreateNtuple("vertices", "Primary vertex data"));

  ana_man->CreateNtupleIColumn(vid, "evtid");
  ana_man->CreateNtupleDColumn(vid, "time_in_ns");
  CreateNtupleFOrDColumn(ana_man, vid, "xloc_in_m", fStoreSinglePrecisionPosition);
  CreateNtupleFOrDColumn(ana_man, vid, "yloc_in_m", fStoreSinglePrecisionPosition);
  CreateNtupleFOrDColumn(ana_man, vid, "zloc_in_m", fStoreSinglePrecisionPosition);
  ana_man->CreateNtupleIColumn(vid, "n_part");

  ana_man->FinishNtuple(vid);

  if (fStorePrimaryParticleInformation) {
    auto pid = RMGManager::Instance()->RegisterNtuple("particles",
        ana_man->CreateNtuple("particles", "Primary particle data"));

    ana_man->CreateNtupleIColumn(pid, "evtid");
    ana_man->CreateNtupleIColumn(pid, "vertexid");
    ana_man->CreateNtupleIColumn(pid, "particle");
    CreateNtupleFOrDColumn(ana_man, pid, "px_in_MeV", fStoreSinglePrecisionEnergy);
    CreateNtupleFOrDColumn(ana_man, pid, "py_in_MeV", fStoreSinglePrecisionEnergy);
    CreateNtupleFOrDColumn(ana_man, pid, "pz_in_MeV", fStoreSinglePrecisionEnergy);
    CreateNtupleFOrDColumn(ana_man, pid, "ekin_in_MeV", fStoreSinglePrecisionEnergy);

    ana_man->FinishNtuple(pid);
  }
}

// invoked in RMGEventAction::EndOfEventAction()
void RMGVertexOutputScheme::StoreEvent(const G4Event* event) {
  if (fSkipPrimaryVertexOutput) return;

  int n_vertex = event->GetNumberOfPrimaryVertex();

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors on primary particles");
    const auto ana_man = G4AnalysisManager::Instance();
    auto vntupleid = rmg_man->GetNtupleID("vertices");
    auto pntupleid = fStorePrimaryParticleInformation ? rmg_man->GetNtupleID("particles") : -1;

    for (int i = 0; i < n_vertex; i++) {
      auto primary_vertex = event->GetPrimaryVertex(i);
      int n_primaries = primary_vertex->GetNumberOfParticle();

      int vcol_id = 0;
      ana_man->FillNtupleIColumn(vntupleid, vcol_id++, event->GetEventID());
      ana_man->FillNtupleDColumn(vntupleid, vcol_id++, primary_vertex->GetT0() / u::ns);
      FillNtupleFOrDColumn(ana_man, vntupleid, vcol_id++, primary_vertex->GetX0() / u::m,
          fStoreSinglePrecisionPosition);
      FillNtupleFOrDColumn(ana_man, vntupleid, vcol_id++, primary_vertex->GetY0() / u::m,
          fStoreSinglePrecisionPosition);
      FillNtupleFOrDColumn(ana_man, vntupleid, vcol_id++, primary_vertex->GetZ0() / u::m,
          fStoreSinglePrecisionPosition);
      ana_man->FillNtupleIColumn(vntupleid, vcol_id++, n_primaries);

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(vntupleid);

      if (fStorePrimaryParticleInformation) {
        for (int j = 0; j < n_primaries; j++) {
          auto primary = primary_vertex->GetPrimary(j);

          int pcol_id = 0;
          ana_man->FillNtupleIColumn(pntupleid, pcol_id++, event->GetEventID());
          ana_man->FillNtupleIColumn(pntupleid, pcol_id++, j);
          ana_man->FillNtupleIColumn(pntupleid, pcol_id++, primary->GetPDGcode());
          FillNtupleFOrDColumn(ana_man, pntupleid, pcol_id++,
              primary->GetMomentum().getX() / u::MeV, fStoreSinglePrecisionEnergy);
          FillNtupleFOrDColumn(ana_man, pntupleid, pcol_id++,
              primary->GetMomentum().getY() / u::MeV, fStoreSinglePrecisionEnergy);
          FillNtupleFOrDColumn(ana_man, pntupleid, pcol_id++,
              primary->GetMomentum().getZ() / u::MeV, fStoreSinglePrecisionEnergy);
          FillNtupleFOrDColumn(ana_man, pntupleid, pcol_id++, primary->GetKineticEnergy() / u::MeV,
              fStoreSinglePrecisionEnergy);

          // NOTE: must be called here for hit-oriented output
          ana_man->AddNtupleRow(pntupleid);
        }
      }
    }
  }
}

void RMGVertexOutputScheme::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Output/Vertex/",
      "Commands for controlling output of primary vertices.");

  fMessenger->DeclareProperty("StorePrimaryParticleInformation", fStorePrimaryParticleInformation)
      .SetGuidance("Store information on primary particle details (not only vertex data).")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("SkipPrimaryVertexOutput", fSkipPrimaryVertexOutput)
      .SetGuidance("Do not store vertex/primary particle data.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionPosition", fStoreSinglePrecisionPosition)
      .SetGuidance("Use float32 (instead of float64) for position output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionEnergy", fStoreSinglePrecisionEnergy)
      .SetGuidance("Use float32 (instead of float64) for energy output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

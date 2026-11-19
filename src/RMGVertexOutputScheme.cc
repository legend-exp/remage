// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <https://www.gnu.org/licenses/>.

#include "RMGVertexOutputScheme.hh"

#include <set>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4TransportationManager.hh"

#include "RMGIpc.hh"
#include "RMGLog.hh"
#include "RMGOutputManager.hh"
#include "RMGOutputTools.hh"

namespace u = CLHEP;

RMGVertexOutputScheme::RMGVertexOutputScheme() { this->DefineCommands(); }

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGVertexOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {
  auto vid = RMGOutputManager::Instance()
                 ->CreateAndRegisterAuxNtuple("vtx", "RMGVertexOutputScheme", ana_man);

  ana_man->CreateNtupleIColumn(vid, "evtid");

  if (!fSkipPrimaryVertexOutput) {
    ana_man->CreateNtupleDColumn(vid, "time_in_ns");
    CreateNtupleFOrDColumn(ana_man, vid, "xloc_in_m", fStoreSinglePrecisionPosition);
    CreateNtupleFOrDColumn(ana_man, vid, "yloc_in_m", fStoreSinglePrecisionPosition);
    CreateNtupleFOrDColumn(ana_man, vid, "zloc_in_m", fStoreSinglePrecisionPosition);
    ana_man->CreateNtupleIColumn(vid, "n_part");
    if (fStoreVolumeInformation) { ana_man->CreateNtupleIColumn(vid, "volumeid"); }
  }

  ana_man->FinishNtuple(vid);

  if (!fSkipPrimaryVertexOutput && fStoreVolumeInformation) {
    auto volid = RMGOutputManager::Instance()
                     ->CreateAndRegisterAuxNtuple("vtx_volumes", "RMGVertexOutputScheme", ana_man);
    ana_man->CreateNtupleIColumn(volid, "volumeid");
    ana_man->CreateNtupleSColumn(volid, "name");
    ana_man->FinishNtuple(volid);
    RMGIpc::SendIpcNonBlocking(RMGIpc::CreateMessage("output_ntuple_deduplicate", "vtx_volumes"));
  }

  if (!fSkipPrimaryVertexOutput && fStorePrimaryParticleInformation) {
    auto pid = RMGOutputManager::Instance()
                   ->CreateAndRegisterAuxNtuple("particles", "RMGVertexOutputScheme", ana_man);

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
  int n_vertex = event->GetNumberOfPrimaryVertex();

  auto rmg_man = RMGOutputManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug_event, "Filling persistent data vectors on primary particles");
    const auto ana_man = G4AnalysisManager::Instance();
    auto vntupleid = rmg_man->GetAuxNtupleID("vtx");
    auto pntupleid = fStorePrimaryParticleInformation ? rmg_man->GetAuxNtupleID("particles") : -1;

    for (int i = 0; i < n_vertex; i++) {
      auto primary_vertex = event->GetPrimaryVertex(i);
      int n_primaries = primary_vertex->GetNumberOfParticle();

      int vcol_id = 0;
      ana_man->FillNtupleIColumn(vntupleid, vcol_id++, GetEventIDForStorage(event));
      if (!fSkipPrimaryVertexOutput) {
        ana_man->FillNtupleDColumn(vntupleid, vcol_id++, primary_vertex->GetT0() / u::ns);
        FillNtupleFOrDColumn(
            ana_man,
            vntupleid,
            vcol_id++,
            primary_vertex->GetX0() / u::m,
            fStoreSinglePrecisionPosition
        );
        FillNtupleFOrDColumn(
            ana_man,
            vntupleid,
            vcol_id++,
            primary_vertex->GetY0() / u::m,
            fStoreSinglePrecisionPosition
        );
        FillNtupleFOrDColumn(
            ana_man,
            vntupleid,
            vcol_id++,
            primary_vertex->GetZ0() / u::m,
            fStoreSinglePrecisionPosition
        );
        ana_man->FillNtupleIColumn(vntupleid, vcol_id++, n_primaries);

        if (fStoreVolumeInformation) {
          int vol_id = -1;
          auto navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
          const auto primary_vol = navigator->LocateGlobalPointAndSetup(primary_vertex->GetPosition());
          if (primary_vol) {
            auto vol_name = primary_vol->GetName();
            auto [it, inserted] = fVolumeMap.try_emplace(vol_name, 0);
            if (inserted) { it->second = RMGOutputTools::HashStringFnv1a(vol_name); }
            vol_id = static_cast<int>(it->second);
          }

          ana_man->FillNtupleIColumn(vntupleid, vcol_id++, vol_id);
        }
      }

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(vntupleid);

      if (!fSkipPrimaryVertexOutput && fStorePrimaryParticleInformation) {
        for (int j = 0; j < n_primaries; j++) {
          auto primary = primary_vertex->GetPrimary(j);

          int pcol_id = 0;
          ana_man->FillNtupleIColumn(pntupleid, pcol_id++, GetEventIDForStorage(event));
          ana_man->FillNtupleIColumn(pntupleid, pcol_id++, j);
          ana_man->FillNtupleIColumn(pntupleid, pcol_id++, primary->GetPDGcode());
          FillNtupleFOrDColumn(
              ana_man,
              pntupleid,
              pcol_id++,
              primary->GetMomentum().getX() / u::MeV,
              fStoreSinglePrecisionEnergy
          );
          FillNtupleFOrDColumn(
              ana_man,
              pntupleid,
              pcol_id++,
              primary->GetMomentum().getY() / u::MeV,
              fStoreSinglePrecisionEnergy
          );
          FillNtupleFOrDColumn(
              ana_man,
              pntupleid,
              pcol_id++,
              primary->GetMomentum().getZ() / u::MeV,
              fStoreSinglePrecisionEnergy
          );
          FillNtupleFOrDColumn(
              ana_man,
              pntupleid,
              pcol_id++,
              primary->GetKineticEnergy() / u::MeV,
              fStoreSinglePrecisionEnergy
          );

          // NOTE: must be called here for hit-oriented output
          ana_man->AddNtupleRow(pntupleid);
        }
      }
    }
  }
}

void RMGVertexOutputScheme::EndOfRunAction(const G4Run*) {
  auto rmg_man = RMGOutputManager::Instance();
  if (!rmg_man->IsPersistencyEnabled() || fSkipPrimaryVertexOutput || !fStoreVolumeInformation)
    return;

  const auto ana_man = G4AnalysisManager::Instance();
  auto ntupleid = rmg_man->GetAuxNtupleID("vtx_volumes");

  std::set<uint32_t> volume_ids; // to check for duplicates.

  for (auto& [vol_name, vol_id_map] : fVolumeMap) {
    int vol_id = static_cast<int>(vol_id_map);

    ana_man->FillNtupleIColumn(ntupleid, 0, vol_id);
    ana_man->FillNtupleSColumn(ntupleid, 1, vol_name);
    ana_man->AddNtupleRow(ntupleid);

    // Check for duplicate volume IDs
    if (!volume_ids.insert(vol_id_map).second) {
      RMGLog::OutDev(RMGLog::error, "Duplicate volume name hash ", vol_id_map);
    }
  }
}

void RMGVertexOutputScheme::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Output/Vertex/",
      "Commands for controlling output of primary vertices."
  );

  fMessenger->DeclareProperty("StorePrimaryParticleInformation", fStorePrimaryParticleInformation)
      .SetGuidance("Store information on primary particle details (not only vertex data).")
      .SetGuidance(
          std::string("This is ") + (fStorePrimaryParticleInformation ? "enabled" : "disabled") +
          " by default"
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("SkipPrimaryVertexOutput", fSkipPrimaryVertexOutput)
      .SetGuidance("Do not store vertex/primary particle data (except the evtid column).")
      .SetGuidance(
          std::string("This is ") + (fSkipPrimaryVertexOutput ? "enabled" : "disabled") + " by default"
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionPosition", fStoreSinglePrecisionPosition)
      .SetGuidance("Use float32 (instead of float64) for position output.")
      .SetGuidance(
          std::string("This is ") + (fStoreSinglePrecisionPosition ? "enabled" : "disabled") +
          " by default"
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionEnergy", fStoreSinglePrecisionEnergy)
      .SetGuidance("Use float32 (instead of float64) for energy output.")
      .SetGuidance(
          std::string("This is ") + (fStoreSinglePrecisionEnergy ? "enabled" : "disabled") + " by default"
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreVolumeInformation", fStoreVolumeInformation)
      .SetGuidance("Store the originating sampling volume.")
      .SetGuidance(
          std::string("This is ") + (fStoreVolumeInformation ? "enabled" : "disabled") + " by default"
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

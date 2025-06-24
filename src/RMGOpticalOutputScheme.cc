// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

#include "RMGOpticalOutputScheme.hh"

#include <set>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCtable.hh"
#include "G4SDManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOpticalDetector.hh"
#include "RMGOutputManager.hh"

namespace u = CLHEP;

RMGOpticalOutputScheme::RMGOpticalOutputScheme() { this->DefineCommands(); }

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGOpticalOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGOutputManager::Instance();
  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  const auto detectors = det_cons->GetDetectorMetadataMap();

  std::set<int> registered_uids;
  std::map<std::string, int> registered_ntuples;
  for (auto&& det : detectors) {
    if (det.second.type != RMGDetectorType::kOptical) continue;

    // do not register the ntuple twice if two detectors share their uid.
    auto had_uid = registered_uids.emplace(det.second.uid);
    if (!had_uid.second) continue;

    auto ntuple_name = this->GetNtupleName(det.second);
    auto ntuple_reg = registered_ntuples.find(ntuple_name);
    if (ntuple_reg != registered_ntuples.end()) {
      // ntuple already exists, but also store the ntuple id for the other uid(s).
      rmg_man->RegisterNtuple(det.second.uid, ntuple_reg->second);
      continue;
    }

    auto id = rmg_man->CreateAndRegisterNtuple(
        det.second.uid,
        ntuple_name,
        "RMGOpticalOutputScheme",
        ana_man
    );
    registered_ntuples.emplace(ntuple_name, id);

    ana_man->CreateNtupleIColumn(id, "evtid");
    if (!fNtuplePerDetector) { ana_man->CreateNtupleIColumn(id, "det_uid"); }
    ana_man->CreateNtupleDColumn(id, "wavelength_in_nm");
    ana_man->CreateNtupleDColumn(id, "time_in_ns");

    ana_man->FinishNtuple(id);
  }
}

// invoked in RMGEventAction::EndOfEventAction()
void RMGOpticalOutputScheme::StoreEvent(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Optical/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Optical/Hits");
    return;
  }

  auto hit_coll = dynamic_cast<RMGOpticalDetectorHitsCollection*>(
      event->GetHCofThisEvent()->GetHC(hit_coll_id)
  );

  if (!hit_coll) {
    RMGLog::Out(RMGLog::error, "Could not find hit collection associated with event");
    return;
  }

  if (hit_coll->entries() <= 0) {
    RMGLog::OutDev(RMGLog::debug, "Hit collection is empty");
    return;
  } else {
    RMGLog::OutDev(RMGLog::debug, "Hit collection contains ", hit_coll->entries(), " hits");
  }

  auto rmg_man = RMGOutputManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();

    for (auto hit : *hit_coll->GetVector()) {
      if (!hit) continue;
      hit->Print();

      auto ntupleid = rmg_man->GetNtupleID(hit->detector_uid);

      int col_id = 0;
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      if (!fNtuplePerDetector) {
        ana_man->FillNtupleIColumn(ntupleid, col_id++, hit->detector_uid);
      }
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->photon_wavelength / u::nm);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_time / u::ns);

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

void RMGOpticalOutputScheme::DefineCommands() {}

// vim: tabstop=2 shiftwidth=2 expandtab

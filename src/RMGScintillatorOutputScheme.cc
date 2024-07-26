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

#include "RMGScintillatorOutputScheme.hh"

#include <set>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGScintillatorDetector.hh"

namespace u = CLHEP;

RMGScintillatorOutputScheme::RMGScintillatorOutputScheme() { this->DefineCommands(); }

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGScintillatorOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGManager::Instance();
  const auto det_cons = rmg_man->GetDetectorConstruction();
  const auto detectors = det_cons->GetDetectorMetadataMap();

  std::set<int> registered_uids;
  std::set<std::string> registered_ntuples;
  for (auto&& det : detectors) {
    if (det.second.type != RMGHardware::kScintillator) continue;

    // do not register the ntuple twice if two detectors share their uid.
    auto had_uid = registered_uids.emplace(det.second.uid);
    if (!had_uid.second) continue;

    auto ntuple_name = this->GetNtupleName(det.second.uid);
    auto had_name = registered_ntuples.emplace(ntuple_name);
    if (!had_name.second) continue;

    auto id =
        rmg_man->RegisterNtuple(det.second.uid, ana_man->CreateNtuple(ntuple_name, "Event data"));

    ana_man->CreateNtupleIColumn(id, "evtid");
    if (!fNtuplePerDetector) { ana_man->CreateNtupleIColumn(id, "det_uid"); }
    ana_man->CreateNtupleIColumn(id, "particle");
    ana_man->CreateNtupleDColumn(id, "edep_in_keV");
    ana_man->CreateNtupleDColumn(id, "time_in_ns");
    ana_man->CreateNtupleDColumn(id, "xloc_pre_in_m");
    ana_man->CreateNtupleDColumn(id, "yloc_pre_in_m");
    ana_man->CreateNtupleDColumn(id, "zloc_pre_in_m");
    ana_man->CreateNtupleDColumn(id, "xloc_post_in_m");
    ana_man->CreateNtupleDColumn(id, "yloc_post_in_m");
    ana_man->CreateNtupleDColumn(id, "zloc_post_in_m");
    ana_man->CreateNtupleDColumn(id, "beta_pre");
    ana_man->CreateNtupleDColumn(id, "beta_post");

    ana_man->FinishNtuple(id);
  }
}

RMGScintillatorDetectorHitsCollection* RMGScintillatorOutputScheme::GetHitColl(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Scintillator/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Scintillator/Hits");
    return nullptr;
  }

  auto hit_coll = dynamic_cast<RMGScintillatorDetectorHitsCollection*>(
      event->GetHCofThisEvent()->GetHC(hit_coll_id));

  if (!hit_coll) {
    RMGLog::Out(RMGLog::error, "Could not find hit collection associated with event");
    return nullptr;
  }

  return hit_coll;
}

// invoked in RMGEventAction::EndOfEventAction()
bool RMGScintillatorOutputScheme::ShouldDiscardEvent(const G4Event* event) {
  // exit fast if no threshold is configured.
  if ((fEdepCutLow < 0 && fEdepCutHigh < 0) || fEdepCutDetectors.empty()) return false;

  auto hit_coll = GetHitColl(event);
  if (!hit_coll) return false;

  // check defined energy threshold.
  double event_edep = 0.;

  for (auto hit : *hit_coll->GetVector()) {
    if (!hit) continue;
    if (fEdepCutDetectors.find(hit->detector_uid) != fEdepCutDetectors.end())
      event_edep += hit->energy_deposition;
  }

  if ((fEdepCutLow > 0 && event_edep < fEdepCutLow) ||
      (fEdepCutHigh > 0 && event_edep > fEdepCutHigh)) {
    RMGLog::Out(RMGLog::debug, "Discarding event - energy threshold has not been met", event_edep,
        fEdepCutLow, fEdepCutHigh);
    return true;
  }

  return false;
}

// invoked in RMGEventAction::EndOfEventAction()
void RMGScintillatorOutputScheme::StoreEvent(const G4Event* event) {
  auto hit_coll = GetHitColl(event);
  if (!hit_coll) return;

  if (hit_coll->entries() <= 0) {
    RMGLog::OutDev(RMGLog::debug, "Hit collection is empty");
    return;
  } else {
    RMGLog::OutDev(RMGLog::debug, "Hit collection contains ", hit_coll->entries(), " hits");
  }

  auto rmg_man = RMGManager::Instance();
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
      ana_man->FillNtupleIColumn(ntupleid, col_id++, hit->particle_type);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->energy_deposition / u::keV);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_time / u::ns);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_position_pre.getX() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_position_pre.getY() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_position_pre.getZ() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_position_post.getX() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_position_post.getY() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_position_post.getZ() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->velocity_pre);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->velocity_post);

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

void RMGScintillatorOutputScheme::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Output/Scintillator/",
      "Commands for controlling output from hits in scintillator detectors.");

  fMessenger
      ->DeclareMethodWithUnit("SetEdepCutLow", "keV", &RMGScintillatorOutputScheme::SetEdepCutLow)
      .SetGuidance("Set a lower energy cut that has to be met for this event to be stored.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit("SetEdepCutHigh", "keV", &RMGScintillatorOutputScheme::SetEdepCutHigh)
      .SetGuidance("Set an upper energy cut that has to be met for this event to be stored.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethod("AddDetectorForEdepThreshold", &RMGScintillatorOutputScheme::AddEdepCutDetector)
      .SetGuidance("Take this detector into account for the filtering by /EdepThreshold.")
      .SetParameterName("det_uid", false)
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

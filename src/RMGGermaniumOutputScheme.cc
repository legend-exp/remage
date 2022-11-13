#include "RMGGermaniumOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCtable.hh"
#include "G4SDManager.hh"

#include "RMGGermaniumDetector.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGGermaniumOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGManager::GetRMGManager();
  const auto det_cons = rmg_man->GetDetectorConstruction();
  const auto detectors = det_cons->GetDetectorMetadataMap();

  for (auto&& det : detectors) {
    if (det.second.type != RMGHardware::kGermanium) continue;

    auto id = rmg_man->RegisterNtuple(det.second.uid);
    ana_man->CreateNtuple(this->GetNtupleName(det.second.uid), "Event data");

    ana_man->CreateNtupleIColumn(id, "evtid");
    ana_man->CreateNtupleDColumn(id, "edep");
    ana_man->CreateNtupleDColumn(id, "time");
    ana_man->CreateNtupleDColumn(id, "xloc");
    ana_man->CreateNtupleDColumn(id, "yloc");
    ana_man->CreateNtupleDColumn(id, "zloc");

    ana_man->FinishNtuple(id);
  }
}

// invoked in RMGEventAction::EndOfEventAction()
void RMGGermaniumOutputScheme::EndOfEventAction(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Germanium/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Germanium/Hits");
    return;
  }

  auto hit_coll = dynamic_cast<RMGGermaniumDetectorHitsCollection*>(
      event->GetHCofThisEvent()->GetHC(hit_coll_id));

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

  auto rmg_man = RMGManager::GetRMGManager();
  if (rmg_man->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");

    for (auto hit : *hit_coll->GetVector()) {
      if (!hit) continue;
      hit->Print();

      const auto evt = rmg_man->GetG4RunManager()->GetCurrentEvent();
      if (!evt) RMGLog::OutDev(RMGLog::fatal, "Current event is nullptr, this should not happen!");

      const auto ana_man = G4AnalysisManager::Instance();
      auto ntupleid = rmg_man->GetNtupleID(hit->detector_uid);

      ana_man->FillNtupleIColumn(ntupleid, 0, evt->GetEventID());
      ana_man->FillNtupleDColumn(ntupleid, 1, hit->energy_deposition / u::keV);
      ana_man->FillNtupleDColumn(ntupleid, 2, hit->global_time / u::ns);
      ana_man->FillNtupleDColumn(ntupleid, 3, hit->global_position.getX() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, 4, hit->global_position.getY() / u::m);
      ana_man->FillNtupleDColumn(ntupleid, 5, hit->global_position.getZ() / u::m);

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

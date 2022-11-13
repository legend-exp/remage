#include "RMGOpticalOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCtable.hh"
#include "G4SDManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOpticalDetector.hh"

namespace u = CLHEP;

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGOpticalOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGManager::GetRMGManager();
  const auto det_cons = rmg_man->GetDetectorConstruction();
  const auto detectors = det_cons->GetDetectorMetadataMap();

  for (auto&& det : detectors) {
    if (det.second.type != RMGHardware::kOptical) continue;

    auto id = rmg_man->RegisterNtuple(det.second.uid);
    ana_man->CreateNtuple(this->GetNtupleName(det.second.uid), "Event data");

    ana_man->CreateNtupleIColumn(id, "evtid");
    ana_man->CreateNtupleDColumn(id, "wavelength");
    ana_man->CreateNtupleDColumn(id, "time");

    ana_man->FinishNtuple(id);
  }
}

// invoked in RMGEventAction::EndOfEventAction()
void RMGOpticalOutputScheme::EndOfEventAction(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Optical/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Optical/Hits");
    return;
  }

  auto hit_coll =
      dynamic_cast<RMGOpticalDetectorHitsCollection*>(event->GetHCofThisEvent()->GetHC(hit_coll_id));

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

      auto ntupleid = rmg_man->GetNtupleID(hit->detector_uid);

      const auto ana_man = G4AnalysisManager::Instance();
      ana_man->FillNtupleIColumn(ntupleid, 0, evt->GetEventID());
      ana_man->FillNtupleDColumn(ntupleid, 1, hit->photon_wavelength / u::nm);
      ana_man->FillNtupleDColumn(ntupleid, 2, hit->global_time / u::ns);

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

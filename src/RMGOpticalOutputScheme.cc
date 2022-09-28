#include "RMGOpticalOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCtable.hh"
#include "G4SDManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOpticalDetector.hh"

void RMGOpticalOutputScheme::clear() {
  detector_uid.clear();
  photon_energies.clear();
}

void RMGOpticalOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {
  ana_man->CreateNtupleIColumn("opt_detid", detector_uid);
  ana_man->CreateNtupleFColumn("opt_ph_ene", photon_energies);
}

void RMGOpticalOutputScheme::EndOfEventAction(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto det_cons = RMGManager::GetRMGManager()->GetDetectorConstruction();
  auto active_dets = det_cons->GetActiveDetectorList();

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

  if (RMGManager::GetRMGManager()->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");

    for (auto hit : *hit_coll->GetVector()) {
      if (!hit) continue;
      hit->Print();

      detector_uid.push_back(hit->GetDetectorUID());
      photon_energies.push_back(hit->GetPhotonEnergy());
    }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include "RMGCalorimeterDetector.hh"

#include <algorithm>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>

#include "G4HCofThisEvent.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOutputTools.hh"


RMGCalorimeterDetector::RMGCalorimeterDetector() : G4VSensitiveDetector("Calorimeter") {

  // declare only one hit collection.
  // NOTE: names in the respective output scheme class must match this
  G4VSensitiveDetector::collectionName.insert("Hits");
}

void RMGCalorimeterDetector::Initialize(G4HCofThisEvent* hit_coll) {

  // create hits collection object
  // NOTE: assumes there is only one collection name (see constructor)
  fHitsCollection = new RMGDetectorHitsCollection(
      G4VSensitiveDetector::SensitiveDetectorName,
      G4VSensitiveDetector::collectionName[0]
  );

  // associate it with the G4HCofThisEvent object
  auto hc_id = G4SDManager::GetSDMpointer()->GetCollectionID(
      G4VSensitiveDetector::SensitiveDetectorName + "/" + G4VSensitiveDetector::collectionName[0]
  );
  hit_coll->AddHitsCollection(hc_id, fHitsCollection);

  // reset the per-detector accumulators for the new event.
  fDetectorHits.clear();
}

bool RMGCalorimeterDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  RMGLog::OutDev(RMGLog::debug, "Processing calorimeter hits");

  // ignore optical photons
  if (step->GetTrack()->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  // we're going to use info from the pre-step point
  const auto prestep = step->GetPreStepPoint();

  // check containment of prestep point
  auto prestep_inside = RMGOutputTools::check_step_point_containment(
      prestep,
      RMGDetectorType::kCalorimeter
  );

  if (not prestep_inside) return false;

  // retrieve unique id for persistency, take from the prestep
  const auto pv = prestep->GetTouchableHandle()->GetVolume();

  auto pv_name = pv->GetName();
  const auto pv_copynr = prestep->GetTouchableHandle()->GetCopyNumber();

  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  auto det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

  RMGLog::OutDev(RMGLog::debug, "Hit in calorimeter nr. ", det_uid, " detected");

  const auto edep = step->GetTotalEnergyDeposit();

  // accumulate energy into one hit per detector. the first step in a detector
  // creates the hit (fixing its physical volume); subsequent steps only add
  // their energy deposit.
  auto it = fDetectorHits.find(det_uid);
  if (it == fDetectorHits.end()) {
    auto* new_hit = new RMGDetectorHit();
    new_hit->detector_uid = det_uid;
    new_hit->physical_volume = pv;
    new_hit->energy_deposition = 0;
    // sentinel: replaced below by the time of the first energy-depositing step.
    new_hit->global_time = std::numeric_limits<double>::max();
    fHitsCollection->insert(new_hit);
    it = fDetectorHits.emplace(det_uid, new_hit).first;
  }

  it->second->energy_deposition += edep;

  // Geant4 does not process steps in time order, and a step may cross the
  // detector without depositing energy. Use the earliest time among the steps
  // that actually deposit energy as the hit time.
  if (edep > 0) {
    it->second->global_time = std::min(it->second->global_time, prestep->GetGlobalTime());
  }

  return true;
}

void RMGCalorimeterDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) {}

// vim: tabstop=2 shiftwidth=2 expandtab

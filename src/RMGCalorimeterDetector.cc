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
}

bool RMGCalorimeterDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  RMGLog::OutDev(RMGLog::debug, "Processing calorimeter hits");

  // return if no energy is deposited
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

  // create a new hit and fill it
  if (fHitsCollection->entries() == 0) {
    auto _hit = new RMGDetectorHit();
    _hit->energy_deposition = 0;
    fHitsCollection->insert(_hit);
  }
  auto* hit = dynamic_cast<RMGDetectorHit*>(fHitsCollection->GetHit(0));

  hit->physical_volume = pv;
  hit->detector_uid = det_uid;
  hit->energy_deposition += step->GetTotalEnergyDeposit();
  hit->global_time = prestep->GetGlobalTime();

  return true;
}

void RMGCalorimeterDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) {}

// vim: tabstop=2 shiftwidth=2 expandtab

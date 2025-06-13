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

#include "RMGGermaniumDetector.hh"

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


RMGGermaniumDetector::RMGGermaniumDetector() : G4VSensitiveDetector("Germanium") {

  // declare only one hit collection.
  // NOTE: names in the respective output scheme class must match this
  G4VSensitiveDetector::collectionName.insert("Hits");
}

void RMGGermaniumDetector::Initialize(G4HCofThisEvent* hit_coll) {

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

bool RMGGermaniumDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  RMGLog::OutDev(RMGLog::debug, "Processing germanium detector hits");

  // return if no energy is deposited
  // ignore optical photons
  if (step->GetTrack()->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  // we're going to use info from the pre-step point
  const auto prestep = step->GetPreStepPoint();
  const auto position_prestep = prestep->GetPosition();

  const auto poststep = step->GetPostStepPoint();
  const auto position_poststep = poststep->GetPosition();
  const auto position_average = (position_prestep + position_poststep) / 2.;

  // check containment of prestep point
  auto prestep_inside = RMGOutputTools::check_step_point_containment(
      prestep,
      RMGDetectorType::kGermanium
  );

  if (not prestep_inside) return false;

  // retrieve unique id for persistency, take from the prestep
  const auto pv = prestep->GetTouchableHandle()->GetVolume();

  auto pv_name = pv->GetName();
  const auto pv_copynr = prestep->GetTouchableHandle()->GetCopyNumber();

  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  auto det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

  RMGLog::OutDev(RMGLog::debug, "Hit in germanium detector nr. ", det_uid, " detected");

  // create a new hit and fill it
  auto* hit = new RMGDetectorHit();

  // pointer to the physical volume
  hit->physical_volume = pv;

  hit->detector_uid = det_uid;
  hit->particle_type = step->GetTrack()->GetDefinition()->GetPDGEncoding();
  hit->energy_deposition = step->GetTotalEnergyDeposit();

  // positions
  hit->global_position_prestep = position_prestep;
  hit->global_position_poststep = position_poststep;
  hit->global_position_average = position_average;

  hit->global_time = prestep->GetGlobalTime();
  hit->track_id = step->GetTrack()->GetTrackID();
  hit->parent_track_id = step->GetTrack()->GetParentID();

  // get various distances
  hit->distance_to_surface_prestep = RMGOutputTools::distance_to_surface(pv, position_prestep);
  hit->distance_to_surface_poststep = RMGOutputTools::distance_to_surface(pv, position_poststep);
  hit->distance_to_surface_average = RMGOutputTools::distance_to_surface(pv, position_average);

  // register the hit in the hit collection for the event
  fHitsCollection->insert(hit);

  return true;
}

void RMGGermaniumDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) {}

// vim: tabstop=2 shiftwidth=2 expandtab

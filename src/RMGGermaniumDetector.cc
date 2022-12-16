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

#include "RMGGermaniumDetector.hh"

#include <map>
#include <stdexcept>
#include <string>

#include "G4AffineTransform.hh"
#include "G4Circle.hh"
#include "G4GenericMessenger.hh"
#include "G4HCofThisEvent.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

G4ThreadLocal G4Allocator<RMGGermaniumDetectorHit>* RMGGermaniumDetectorHitAllocator = nullptr;

// NOTE: does this make sense?
G4bool RMGGermaniumDetectorHit::operator==(const RMGGermaniumDetectorHit& right) const {
  return (this == &right) ? true : false;
}

void RMGGermaniumDetectorHit::Print() {
  // TODO: add all fields
  RMGLog::Out(RMGLog::debug, "Detector UID: ", this->detector_uid,
      " / Energy: ", G4BestUnit(this->energy_deposition, "Energy"),
      " / Position: ", this->global_position / CLHEP::m, " m",
      " / Time: ", this->global_time / CLHEP::ns, " ns");
}

void RMGGermaniumDetectorHit::Draw() {
  const auto vis_man = G4VVisManager::GetConcreteInstance();
  if (vis_man and this->energy_deposition > 0) {
    G4Circle circle(this->global_position);
    circle.SetScreenSize(5);
    circle.SetFillStyle(G4Circle::filled);
    circle.SetVisAttributes(G4VisAttributes(G4Colour(1, 0, 0)));
    vis_man->Draw(circle);
  }
}

RMGGermaniumDetector::RMGGermaniumDetector() : G4VSensitiveDetector("Germanium") {

  // declare only one hit collection.
  // NOTE: names in the respective output scheme class must match this
  G4VSensitiveDetector::collectionName.insert("Hits");

  this->DefineCommands();
}

void RMGGermaniumDetector::Initialize(G4HCofThisEvent* hit_coll) {

  // create hits collection object
  // NOTE: assumes there is only one collection name (see constructor)
  fHitsCollection =
      new RMGGermaniumDetectorHitsCollection(G4VSensitiveDetector::SensitiveDetectorName,
          G4VSensitiveDetector::collectionName[0]);

  // associate it with the G4HCofThisEvent object
  auto hc_id = G4SDManager::GetSDMpointer()->GetCollectionID(G4VSensitiveDetector::collectionName[0]);
  hit_coll->AddHitsCollection(hc_id, fHitsCollection);
}

bool RMGGermaniumDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  RMGLog::OutDev(RMGLog::debug, "Processing germanium detector hits");

  // return if no energy is deposited
  if (step->GetTotalEnergyDeposit() == 0) return false;
  // ignore optical photons
  if (step->GetTrack()->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  // we're going to use info from the pre-step point
  const auto prestep = step->GetPreStepPoint();

  // locate us
  const auto pv_name = prestep->GetTouchableHandle()->GetVolume()->GetName();
  const auto pv_copynr = prestep->GetTouchableHandle()->GetCopyNumber();

  // check if physical volume is registered as germanium detector
  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  try {
    auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
    if (d_type != RMGHardware::kGermanium) {
      RMGLog::OutFormatDev(RMGLog::debug,
          "Volume '{}' (copy nr. {} not registered as germanium detector", pv_name, pv_copynr);
      return false;
    }
  } catch (const std::out_of_range& e) {
    RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {}) not registered as detector",
        pv_name, pv_copynr);
    return false;
  }

  // retrieve unique id for persistency
  auto det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

  RMGLog::OutDev(RMGLog::debug, "Hit in germanium detector nr. ", det_uid, " detected");

  // create a new hit and fill it
  RMGGermaniumDetectorHit* hit = new RMGGermaniumDetectorHit();
  hit->detector_uid = det_uid;
  hit->energy_deposition = step->GetTotalEnergyDeposit();
  hit->global_position = prestep->GetPosition();
  hit->global_time = prestep->GetGlobalTime();

  // register the hit in the hit collection for the event
  fHitsCollection->insert(hit);

  return true;
}

void RMGGermaniumDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) { return; }

void RMGGermaniumDetector::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Detector/Germanium/",
      "Commands for controlling stuff");
}

// vim: tabstop=2 shiftwidth=2 expandtab

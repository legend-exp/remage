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

#include "RMGScintillatorDetector.hh"

#include <map>
#include <stdexcept>
#include <string>

#include "G4Circle.hh"
#include "G4HCofThisEvent.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

G4ThreadLocal G4Allocator<RMGScintillatorDetectorHit>* RMGScintillatorDetectorHitAllocator = nullptr;

// NOTE: does this make sense?
G4bool RMGScintillatorDetectorHit::operator==(const RMGScintillatorDetectorHit& right) const {
  return this == &right;
}

void RMGScintillatorDetectorHit::Print() {
  RMGLog::Out(RMGLog::debug, "Detector UID: ", this->detector_uid,
      " / Particle: ", this->particle_type,
      " / Energy: ", G4BestUnit(this->energy_deposition, "Energy"),
      " / Position: ", this->global_position_pre / CLHEP::m, " m",
      " / Time: ", this->global_time / CLHEP::ns, " ns");
}

void RMGScintillatorDetectorHit::Draw() {
  const auto vis_man = G4VVisManager::GetConcreteInstance();
  if (vis_man and this->energy_deposition > 0) {
    G4Circle circle(this->global_position_pre);
    circle.SetScreenSize(5);
    circle.SetFillStyle(G4Circle::filled);
    circle.SetVisAttributes(G4VisAttributes(G4Colour(0, 0, 1)));
    vis_man->Draw(circle);
  }
}

RMGScintillatorDetector::RMGScintillatorDetector() : G4VSensitiveDetector("Scintillator") {

  // declare only one hit collection.
  // NOTE: names in the respective output scheme class must match this
  G4VSensitiveDetector::collectionName.insert("Hits");
}

void RMGScintillatorDetector::Initialize(G4HCofThisEvent* hit_coll) {

  // create hits collection object
  // NOTE: assumes there is only one collection name (see constructor)
  fHitsCollection =
      new RMGScintillatorDetectorHitsCollection(G4VSensitiveDetector::SensitiveDetectorName,
          G4VSensitiveDetector::collectionName[0]);

  // associate it with the G4HCofThisEvent object
  auto hc_id = G4SDManager::GetSDMpointer()->GetCollectionID(
      G4VSensitiveDetector::SensitiveDetectorName + "/" + G4VSensitiveDetector::collectionName[0]);
  hit_coll->AddHitsCollection(hc_id, fHitsCollection);
}

bool RMGScintillatorDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  RMGLog::OutDev(RMGLog::debug, "Processing scintillator detector hits");

  // return if no energy is deposited
  if (step->GetTotalEnergyDeposit() == 0) return false;
  // ignore optical photons
  if (step->GetTrack()->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  // we're going to use info from the pre-step point
  const auto prestep = step->GetPreStepPoint();
  const auto poststep = step->GetPostStepPoint();

  // locate us
  const auto pv_name = prestep->GetTouchableHandle()->GetVolume()->GetName();
  const auto pv_copynr = prestep->GetTouchableHandle()->GetCopyNumber();

  // check if physical volume is registered as sermanium detector
  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  try {
    auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
    if (d_type != RMGHardware::kScintillator) {
      RMGLog::OutFormatDev(RMGLog::debug,
          "Volume '{}' (copy nr. {} not registered as scintillator detector", pv_name, pv_copynr);
      return false;
    }
  } catch (const std::out_of_range& e) {
    RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {}) not registered as detector",
        pv_name, pv_copynr);
    return false;
  }

  // retrieve unique id for persistency
  auto det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

  RMGLog::OutDev(RMGLog::debug, "Hit in scintillator detector nr. ", det_uid, " detected");

  // create a new hit and fill it
  auto* hit = new RMGScintillatorDetectorHit();
  hit->detector_uid = det_uid;
  hit->particle_type = step->GetTrack()->GetDefinition()->GetPDGEncoding();
  hit->energy_deposition = step->GetTotalEnergyDeposit();
  hit->global_position_pre = prestep->GetPosition();
  hit->global_position_post = poststep->GetPosition();
  hit->global_time = prestep->GetGlobalTime();
  hit->velocity_pre = prestep->GetVelocity();
  hit->velocity_post = poststep->GetVelocity();

  // register the hit in the hit collection for the event
  fHitsCollection->insert(hit);

  return true;
}

void RMGScintillatorDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) {}

// vim: tabstop=2 shiftwidth=2 expandtab

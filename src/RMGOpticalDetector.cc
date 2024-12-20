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

#include "RMGOpticalDetector.hh"

#include <map>
#include <stdexcept>

#include "G4HCofThisEvent.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4VVisManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

G4ThreadLocal G4Allocator<RMGOpticalDetectorHit>* RMGOpticalDetectorHitAllocator = nullptr;

// NOTE: does this make sense?
G4bool RMGOpticalDetectorHit::operator==(const RMGOpticalDetectorHit& right) const {
  return this == &right;
}

void RMGOpticalDetectorHit::Print() {
  RMGLog::OutFormat(RMGLog::debug, "Detector UID: {} / Wavelength: {} nm", this->detector_uid,
      this->photon_wavelength / CLHEP::nm);
}

void RMGOpticalDetectorHit::Draw() {

  const auto vis_man = G4VVisManager::GetConcreteInstance();
  if (!vis_man || !detector_touchable) return;

  auto lv_va = detector_touchable->GetVolume()->GetLogicalVolume()->GetVisAttributes();
  G4VisAttributes va;
  if (lv_va) va = *lv_va;
  va.SetColour(G4Colour(0, 0, 1));
  va.SetForceSolid(true);

  auto pos = detector_touchable->GetTranslation(detector_touchable->GetHistoryDepth());
  auto rot = detector_touchable->GetRotation(detector_touchable->GetHistoryDepth());

  vis_man->Draw(*detector_touchable->GetVolume(), va, G4Transform3D(*rot, pos));
}

RMGOpticalDetector::RMGOpticalDetector() : G4VSensitiveDetector("Optical") {

  // declare only one hit collection.
  // NOTE: names in the respective output scheme class must match this
  G4VSensitiveDetector::collectionName.insert("Hits");
}

void RMGOpticalDetector::Initialize(G4HCofThisEvent* hit_coll) {

  // create hits collection object
  // NOTE: assumes there is only one collection name (see constructor)
  fHitsCollection = new RMGOpticalDetectorHitsCollection(G4VSensitiveDetector::SensitiveDetectorName,
      G4VSensitiveDetector::collectionName[0]);

  // associate it with the G4HCofThisEvent object
  auto hc_id = G4SDManager::GetSDMpointer()->GetCollectionID(
      G4VSensitiveDetector::SensitiveDetectorName + "/" + G4VSensitiveDetector::collectionName[0]);
  hit_coll->AddHitsCollection(hc_id, fHitsCollection);
}

bool RMGOpticalDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  RMGLog::OutDev(RMGLog::debug, "Processing optical detector hits");

  // optical photon?
  auto particle = step->GetTrack()->GetDefinition();
  if (particle != G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  // this is actually irrelevant as optical photons do not truly carry the energy deposited
  // This yields the photon wavelength (in energy units)
  if (step->GetTotalEnergyDeposit() == 0) return false;

  // Get the physical volume of the detection point (post step). A step starts
  // at PreStepPoint and ends at PostStepPoint. If a boundary is reached, the
  // PostStepPoint belongs logically to the next volume. As we write down the
  // hit when the photon reaches the boundary we need to check the
  // PostStepPoint here
  auto touchable = step->GetPostStepPoint()->GetTouchableHandle();
  const auto pv_name = touchable->GetVolume()->GetName();
  const auto pv_copynr = touchable->GetCopyNumber();

  // check if physical volume is registered as optical detector
  auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  try {
    auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
    if (d_type != RMGDetectorType::kOptical) {
      RMGLog::OutFormatDev(RMGLog::debug,
          "Volume '{}' (copy nr. {} not registered as optical detector", pv_name, pv_copynr);
      return false;
    }
  } catch (const std::out_of_range& e) {
    RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {} not registered as detector",
        pv_name, pv_copynr);
    return false;
  }

  // retrieve unique id for persistency
  auto det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

  RMGLog::OutDev(RMGLog::debug, "Hit in optical detector nr. ", det_uid, " detected");

  // initialize hit object for uid, if not already there
  auto* hit = new RMGOpticalDetectorHit();
  if (G4VVisManager::GetConcreteInstance()) { hit->detector_touchable = touchable; }
  hit->detector_uid = det_uid;
  hit->photon_wavelength = CLHEP::c_light * CLHEP::h_Planck / step->GetTotalEnergyDeposit();
  hit->global_time = step->GetPostStepPoint()->GetGlobalTime();

  // register the hit in the hit collection for the event
  fHitsCollection->insert(hit);

  return true;
}

void RMGOpticalDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) {}

// vim: tabstop=2 shiftwidth=2 expandtab

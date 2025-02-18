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
#include "G4HCofThisEvent.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

/// \cond this triggers a sphinx error
G4ThreadLocal G4Allocator<RMGGermaniumDetectorHit>* RMGGermaniumDetectorHitAllocator = nullptr;
/// \endcond

// NOTE: does this make sense?
G4bool RMGGermaniumDetectorHit::operator==(const RMGGermaniumDetectorHit& right) const {
  return this == &right;
}

void RMGGermaniumDetectorHit::Print() {
  RMGLog::Out(RMGLog::debug, "Detector UID: ", this->detector_uid,
      " / Particle: ", this->particle_type,
      " / Energy: ", G4BestUnit(this->energy_deposition, "Energy"),
      " / Position (prestep): ", this->global_position_prestep / CLHEP::m, " m",
      " / Time: ", this->global_time / CLHEP::ns, " ns");
}

void RMGGermaniumDetectorHit::Draw() {
  const auto vis_man = G4VVisManager::GetConcreteInstance();
  if (vis_man and this->energy_deposition > 0) {
    G4Circle circle(this->global_position_prestep);
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
}

void RMGGermaniumDetector::Initialize(G4HCofThisEvent* hit_coll) {

  // create hits collection object
  // NOTE: assumes there is only one collection name (see constructor)
  fHitsCollection =
      new RMGGermaniumDetectorHitsCollection(G4VSensitiveDetector::SensitiveDetectorName,
          G4VSensitiveDetector::collectionName[0]);

  // associate it with the G4HCofThisEvent object
  auto hc_id = G4SDManager::GetSDMpointer()->GetCollectionID(
      G4VSensitiveDetector::SensitiveDetectorName + "/" + G4VSensitiveDetector::collectionName[0]);
  hit_coll->AddHitsCollection(hc_id, fHitsCollection);
}

double RMGGermaniumDetector::DistanceToSurface(const G4VPhysicalVolume* pv,
    const G4ThreeVector& position) {

  // get logical volume and solid
  auto pv_name = pv->GetName();
  const auto lv = pv->GetLogicalVolume();
  const auto sv = lv->GetSolid();

  // get translation
  G4AffineTransform tf(pv->GetRotation(), pv->GetTranslation());
  tf.Invert();

  // Get distance to surface.
  // First transform coordinates into local system

  double dist = sv->DistanceToOut(tf.TransformPoint(position));

  // Also check distance to daughters if there are any. Analogue to G4NormalNavigation.cc
  auto local_no_daughters = lv->GetNoDaughters();
  // increase by one to keep positive in reverse loop.
  for (auto sample_no = local_no_daughters; sample_no >= 1; sample_no--) {
    const auto sample_physical = lv->GetDaughter(sample_no - 1);
    G4AffineTransform sample_tf(sample_physical->GetRotation(), sample_physical->GetTranslation());
    sample_tf.Invert();
    const auto sample_point = sample_tf.TransformPoint(position);
    const auto sample_solid = sample_physical->GetLogicalVolume()->GetSolid();
    const double sample_dist = sample_solid->DistanceToIn(sample_point);
    if (sample_dist < dist) { dist = sample_dist; }
  }
  return dist;
}

bool RMGGermaniumDetector::CheckStepPointContainment(const G4StepPoint* step_point) {

  const auto pv = step_point->GetTouchableHandle()->GetVolume();
  auto pv_name = pv->GetName();
  const auto pv_copynr = step_point->GetTouchableHandle()->GetCopyNumber();
  const auto lv = pv->GetLogicalVolume();
  const auto sv = lv->GetSolid();

  // check if physical volume is registered as germanium detector
  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  try {
    auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
    if (d_type != RMGDetectorType::kGermanium) {
      RMGLog::OutFormatDev(RMGLog::debug,
          "Volume '{}' (copy nr. {} not registered as germanium detector", pv_name, pv_copynr);
      return false;
    }
  } catch (const std::out_of_range& e) {
    RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {}) not registered as detector",
        pv_name, pv_copynr);
    return false;
  }
  return true;
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

  // check containment of pre and post-step point
  auto prestep_inside = this->CheckStepPointContainment(prestep);
  // auto poststep_inside = this->CheckStepPointContainment(poststep);

  if (not prestep_inside) return false;

  // retrieve unique id for persistency, take from the prestep
  const auto pv = prestep->GetTouchableHandle()->GetVolume();
  auto pv_name = pv->GetName();
  const auto pv_copynr = prestep->GetTouchableHandle()->GetCopyNumber();

  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  auto det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

  RMGLog::OutDev(RMGLog::debug, "Hit in germanium detector nr. ", det_uid, " detected");

  // create a new hit and fill it
  auto* hit = new RMGGermaniumDetectorHit();
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
  hit->distance_to_surface_prestep = DistanceToSurface(pv, position_prestep);
  hit->distance_to_surface_poststep = DistanceToSurface(pv, position_poststep);
  hit->distance_to_surface_average = DistanceToSurface(pv, position_average);

  // register the hit in the hit collection for the event
  fHitsCollection->insert(hit);

  return true;
}

void RMGGermaniumDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) {}

// vim: tabstop=2 shiftwidth=2 expandtab

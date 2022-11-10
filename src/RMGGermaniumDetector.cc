#include "RMGGermaniumDetector.hh"

#include <map>
#include <stdexcept>

#include "G4AffineTransform.hh"
#include "G4GenericMessenger.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

G4ThreadLocal G4Allocator<RMGGermaniumDetectorHit>* RMGGermaniumDetectorHitAllocator = nullptr;

// NOTE: does this make sense?
G4bool RMGGermaniumDetectorHit::operator==(const RMGGermaniumDetectorHit& right) const {
  return (this == &right) ? true : false;
}

void RMGGermaniumDetectorHit::Print() {
  RMGLog::OutFormat(RMGLog::debug, "Detector UID: {} / Energy: {}", this->detector_uid,
      this->energy_deposition);
}

void RMGGermaniumDetectorHit::Draw() {
  auto vis_man = G4VVisManager::GetConcreteInstance();
  if (vis_man) {
    G4Circle circle(position);
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

  if (step->GetTotalEnergyDeposit() <= 0) return false;

  // Get the physical volume of the detection point (post step). A step starts
  // at PreStepPoint and ends at PostStepPoint. If a boundary is reached, the
  // PostStepPoint belongs logically to the next volume. As we write down the
  // hit when the particle reaches the boundary we need to check the
  // PostStepPoint here
  const auto pv_name = step->GetPostStepPoint()->GetPhysicalVolume()->GetName();
  const auto pv_copynr = step->GetPostStepPoint()->GetTouchableHandle()->GetCopyNumber();

  // check if physical volume is registered as germanium detector
  auto det_cons = RMGManager::GetRMGManager()->GetDetectorConstruction();
  try {
    auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
    if (d_type != RMGHardware::kGermanium) {
      RMGLog::OutFormatDev(RMGLog::debug,
          "Volume '{}' (copy nr. {} not registered as germanium detector", pv_name, pv_copynr);
      return false;
    }
  } catch (const std::out_of_range& e) {
    RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {} not registered as detector",
        pv_name, pv_copynr);
    return false;
  }

  // retrieve unique id for persistency
  auto det_uid = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).uid;

  RMGLog::OutDev(RMGLog::debug, "Hit in germanium detector nr. ", det_uid, " detected");

  RMGGermaniumDetectorHit* hit = new RMGGermaniumDetectorHit();
  hit->detector_uid = det_uid;
  hit->energy_deposition = step->GetTotalEnergyDeposit() / CLHEP::keV;
  hit->position = step->GetPreStepPoint()->GetPosition();
  fHitsCollection->insert(hit);

  return true;
}

void RMGGermaniumDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) { return; }

void RMGGermaniumDetector::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Detector/Germanium/",
      "Commands for controlling stuff");
}

// vim: tabstop=2 shiftwidth=2 expandtab

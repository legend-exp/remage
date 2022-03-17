#include "RMGOpticalDetector.hh"

#include <stdexcept>
#include <map>

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ThreeVector.hh"
#include "G4GenericMessenger.hh"
#include "G4OpticalPhoton.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGDetectorConstruction.hh"

G4ThreadLocal G4Allocator<RMGOpticalDetectorHit>* RMGOpticalDetectorHitAllocator = nullptr;

// NOTE: does this make sense?
G4bool RMGOpticalDetectorHit::operator==(const RMGOpticalDetectorHit& right) const {
  return ( this == &right ) ? true : false;
}

void RMGOpticalDetectorHit::Print() {
  RMGLog::Out(RMGLog::debug, "Detector UID: ", fDetectorUID);
  RMGLog::Out(RMGLog::debug, "Number of photoelectrons: ", fPhotoElectrons);
}

RMGOpticalDetector::RMGOpticalDetector() :
  G4VSensitiveDetector("Optical") {

  // declare only one hit collection.
  // NOTE: names in the respective output scheme class must match this
  G4VSensitiveDetector::collectionName.insert("Hits");

  this->DefineCommands();
}

void RMGOpticalDetector::Initialize(G4HCofThisEvent* hit_coll) {

  // create hits collection object
  // NOTE: assumes there is only one collection name (see constructor)
  fHitsCollection = new RMGOpticalDetectorHitsCollection(
      G4VSensitiveDetector::SensitiveDetectorName,
      G4VSensitiveDetector::collectionName[0]);

  // associate it with the G4HCofThisEvent object
  auto hc_id = G4SDManager::GetSDMpointer()->GetCollectionID(G4VSensitiveDetector::collectionName[0]);
  hit_coll->AddHitsCollection(hc_id, fHitsCollection);
}

bool RMGOpticalDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  RMGLog::OutDev(RMGLog::debug, "Processing optical detector hits");

  // optical photon?
  auto particle = step->GetTrack()->GetDefinition();
  if (particle != G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  // this is actually irrelevant as optical photons do not truly carry the energy deposited
  if (step->GetTotalEnergyDeposit() <= 0) return false; // This yields the photon wavelength (in energy units)

  // Get the physical volume of the detection point (post step). A step starts
  // at PreStepPoint and ends at PostStepPoint. If a boundary is reached, the
  // PostStepPoint belongs logically to the next volume. As we write down the
  // hit when the photon reaches the boundary we need to check the
  // PostStepPoint here
  const auto pv_name = step->GetPostStepPoint()->GetPhysicalVolume()->GetName();
  const auto pv_copynr = step->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber();

  // check if physical volume is registered as optical detector
  auto det_cons = RMGManager::GetRMGManager()->GetDetectorConstruction();
  try {
    auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
    if (d_type != RMGDetectorConstruction::kOptical) {
      RMGLog::OutFormatDev(RMGLog::debug, "Volume '{}' (copy nr. {} not registered as optical detector",
          pv_name, pv_copynr);
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

  RMGOpticalDetectorHit* hit = nullptr;

  // initialize hit object for uid, if not already there
  const auto& hit_vec = fHitsCollection->GetVector();
  const auto& result = std::find_if(hit_vec->begin(), hit_vec->end(),
      [&det_uid](RMGOpticalDetectorHit* h){ return h->GetDetectorUID() == (int)det_uid; });

  if (result == hit_vec->end()) {
    RMGLog::OutDev(RMGLog::debug, "No hit object found, initializing");
    hit = new RMGOpticalDetectorHit();
    hit->SetDetectorUID(det_uid);
    fHitsCollection->insert(hit);
  }
  else hit = *result;

  // integrate hit info
  RMGLog::OutDev(RMGLog::debug, "Adding hit data to detector hit container");
  hit->AddPhotoElectron();

  return true;
}

void RMGOpticalDetector::EndOfEvent(G4HCofThisEvent* /*hit_coll*/) {
  return;
}

void RMGOpticalDetector::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Detector/Optical/",
      "Commands for controlling stuff");
}

// vim: tabstop=2 shiftwidth=2 expandtab

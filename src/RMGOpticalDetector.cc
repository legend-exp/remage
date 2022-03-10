#include "RMGOpticalDetector.hh"

#include <map>

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ThreeVector.hh"
#include "G4GenericMessenger.hh"

#include "RMGOpticalDetectorHit.hh"
#include "RMGLog.hh"

RMGOpticalDetector::RMGOpticalDetector(const std::string& name, const std::string& hits_coll_name) :
  G4VSensitiveDetector(name) {

  G4VSensitiveDetector::collectionName.insert(hits_coll_name);

  this->DefineCommands();
}

void RMGOpticalDetector::Initialize(G4HCofThisEvent* hit_coll) {

  // create hits collection object
  fHitsCollection = new RMGOpticalDetectorHitsCollection(
      G4VSensitiveDetector::SensitiveDetectorName,
      G4VSensitiveDetector::collectionName[0]);

  // associate it with the G4HofThisEvent object
  auto hc_id = G4SDManager::GetSDMpointer()->GetCollectionID(G4VSensitiveDetector::collectionName[0]);
  hit_coll->AddHitsCollection(hc_id, fHitsCollection);
}

bool RMGOpticalDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {

  // which detector is this? Use copy number
  const size_t det_id = step->GetPreStepPoint()->GetTouchable()->GetCopyNumber();

  RMGLog::Out(RMGLog::debug, "Hit in optical detector nr. ", det_id, " detected");

  RMGOpticalDetectorHit* hit = nullptr;

  const auto& hit_vec = fHitsCollection->GetVector();
  const auto& result = std::find_if(hit_vec->begin(), hit_vec->end(),
      [&det_id](RMGOpticalDetectorHit* h){ return h->GetDetectorID() == (int)det_id; });

  if (result == hit_vec->end()) {
    RMGLog::Out(RMGLog::debug, "No hit object found, initializing");
    hit = new RMGOpticalDetectorHit();
    fHitsCollection->insert(hit);
  }
  else hit = *result;

  // integrate hit info
  RMGLog::Out(RMGLog::debug, "Adding hit data to detector hit container");
  hit->SetDetectorID(det_id);

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

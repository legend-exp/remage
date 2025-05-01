
#include "IsotopeOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

void IsotopeOutputScheme::ClearBeforeEvent() {
  zOfEvent.clear();
  aOfEvent.clear();
}

// invoked in RMGRunAction::SetupAnalysisManager()
void IsotopeOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGManager::Instance();

  auto id = rmg_man->RegisterNtuple(
      OutputRegisterID,
      ana_man->CreateNtuple("IsotopeOutput", "Event data")
  );

  ana_man->CreateNtupleIColumn(id, "evtid");
  // Could be done with particle PDG but this way it is human readable
  ana_man->CreateNtupleIColumn(id, "Z");
  ana_man->CreateNtupleIColumn(id, "A");

  ana_man->FinishNtuple(id);
}

void IsotopeOutputScheme::TrackingActionPre(const G4Track* aTrack) {
  const auto particle = aTrack->GetParticleDefinition();
  // if (!particle->IsGeneralIon()) return;
  const int z = particle->GetAtomicNumber();
  const int a = particle->GetAtomicMass();
  if (z == 0 || a == 0) return; // not an isotope, but any other particle.

  G4String CreatorName = aTrack->GetCreatorProcess()->GetProcessName();

  // In Theory there should always be only one capture per event. But the more general approach is to be able to store multiple
  if (CreatorName == "RMGnCapture" || CreatorName == "nCapture") {
    zOfEvent.push_back(z);
    aOfEvent.push_back(a);
  }
}

// invoked in RMGEventAction::EndOfEventAction()
void IsotopeOutputScheme::StoreEvent(const G4Event* event) {
  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();

    for (size_t i = 0; i < zOfEvent.size(); ++i) {
      auto ntupleid = rmg_man->GetNtupleID(OutputRegisterID);

      int col_id = 0;
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      ana_man->FillNtupleIColumn(ntupleid, col_id++, zOfEvent[i]);
      ana_man->FillNtupleIColumn(ntupleid, col_id++, aOfEvent[i]);

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include "RMGScintillatorOutputScheme.hh"

#include <memory>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"

#include "RMGHardware.hh"
#include "RMGIpc.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGScintillatorDetector.hh"
#include "RMGTools.hh"

namespace u = CLHEP;

RMGScintillatorOutputScheme::RMGScintillatorOutputScheme() {

  // set default clustering parameters
  fPreClusterPars.cluster_time_threshold = 10 * u::us;
  fPreClusterPars.cluster_distance = 500 * u::um;
  fPreClusterPars.track_energy_threshold = 10 * u::keV;
  fPreClusterPars.combine_low_energy_tracks = true;
  fPreClusterPars.reassign_gamma_energy = true;

  this->DefineCommands();
}


// invoked in RMGRunAction::SetupAnalysisManager()
void RMGScintillatorOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGManager::Instance();
  const auto det_cons = rmg_man->GetDetectorConstruction();
  const auto detectors = det_cons->GetDetectorMetadataMap();

  std::set<int> registered_uids;
  std::map<std::string, int> registered_ntuples;
  for (auto&& det : detectors) {
    if (det.second.type != RMGDetectorType::kScintillator) continue;

    // do not register the ntuple twice if two detectors share their uid.
    auto had_uid = registered_uids.emplace(det.second.uid);
    if (!had_uid.second) continue;

    auto ntuple_name = this->GetNtupleName(det.second);
    auto ntuple_reg = registered_ntuples.find(ntuple_name);
    if (ntuple_reg != registered_ntuples.end()) {
      // ntuple already exists, but also store the ntuple id for the other uid(s).
      rmg_man->RegisterNtuple(det.second.uid, ntuple_reg->second);
      continue;
    }

    auto id = rmg_man->RegisterNtuple(det.second.uid, ana_man->CreateNtuple(ntuple_name, "Event data"));
    registered_ntuples.emplace(ntuple_name, id);
    RMGIpc::SendIpcNonBlocking(
        RMGIpc::CreateMessage("output_table", std::string("scintillator\x1e") + ntuple_name)
    );

    ana_man->CreateNtupleIColumn(id, "evtid");
    if (!fNtuplePerDetector) { ana_man->CreateNtupleIColumn(id, "det_uid"); }
    ana_man->CreateNtupleIColumn(id, "particle");

    if (fStoreTrackID) {
      ana_man->CreateNtupleIColumn(id, "trackid");
      ana_man->CreateNtupleIColumn(id, "parent_trackid");
    }
    CreateNtupleFOrDColumn(ana_man, id, "edep_in_keV", fStoreSinglePrecisionEnergy);

    ana_man->CreateNtupleDColumn(id, "time_in_ns");

    CreateNtupleFOrDColumn(ana_man, id, "xloc_in_m", fStoreSinglePrecisionPosition);
    CreateNtupleFOrDColumn(ana_man, id, "yloc_in_m", fStoreSinglePrecisionPosition);
    CreateNtupleFOrDColumn(ana_man, id, "zloc_in_m", fStoreSinglePrecisionPosition);

    // save also a second position if requested
    if (fPositionMode == RMGOutputTools::PositionMode::kBoth) {

      CreateNtupleFOrDColumn(ana_man, id, "xloc_pre_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "yloc_pre_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "zloc_pre_in_m", fStoreSinglePrecisionPosition);

      CreateNtupleFOrDColumn(ana_man, id, "xloc_post_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "yloc_post_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "zloc_post_in_m", fStoreSinglePrecisionPosition);
    }
    if (fStoreVelocity) {
      CreateNtupleFOrDColumn(ana_man, id, "v_pre_in_m\\ns", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "v_post_in_m\\ns", fStoreSinglePrecisionPosition);
    }
    ana_man->FinishNtuple(id);
  }
}

RMGDetectorHitsCollection* RMGScintillatorOutputScheme::GetHitColl(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Scintillator/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Scintillator/Hits");
    return nullptr;
  }

  auto hit_coll = dynamic_cast<RMGDetectorHitsCollection*>(
      event->GetHCofThisEvent()->GetHC(hit_coll_id)
  );

  if (!hit_coll) {
    RMGLog::Out(RMGLog::error, "Could not find hit collection associated with event");
    return nullptr;
  }

  return hit_coll;
}

bool RMGScintillatorOutputScheme::ShouldDiscardEvent(const G4Event* event) {
  // exit fast if no threshold is configured.
  if ((fEdepCutLow < 0 && fEdepCutHigh < 0) || fEdepCutDetectors.empty()) return false;

  auto hit_coll = GetHitColl(event);
  if (!hit_coll) return false;

  // check defined energy threshold.
  double event_edep = 0.;

  for (auto hit : *hit_coll->GetVector()) {
    if (!hit) continue;
    if (fEdepCutDetectors.find(hit->detector_uid) != fEdepCutDetectors.end())
      event_edep += hit->energy_deposition;
  }

  if ((fEdepCutLow > 0 && event_edep < fEdepCutLow) ||
      (fEdepCutHigh > 0 && event_edep > fEdepCutHigh)) {
    RMGLog::Out(
        RMGLog::debug,
        "Discarding event - energy threshold has not been met",
        event_edep,
        fEdepCutLow,
        fEdepCutHigh
    );
    return true;
  }

  return false;
}

void RMGScintillatorOutputScheme::StoreEvent(const G4Event* event) {
  auto hit_coll = GetHitColl(event);

  if (!hit_coll) return;

  if (hit_coll->entries() <= 0) {
    RMGLog::OutDev(RMGLog::debug, "Hit collection is empty");
    return;
  } else {
    RMGLog::OutDev(RMGLog::debug, "Hit collection contains ", hit_coll->entries(), " hits");
  }

  // pre-cluster the hits if requested
  std::shared_ptr<RMGDetectorHitsCollection> _clustered_hits;
  if (fPreClusterHits) {
    _clustered_hits = RMGOutputTools::pre_cluster_hits(hit_coll, fPreClusterPars, false, true);
    hit_coll = _clustered_hits.get(); // get an unmanaged ptr for use in this function
  }

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();

    for (auto hit : *hit_coll->GetVector()) {
      if (!hit or (hit->energy_deposition == 0 and this->fDiscardZeroEnergyHits)) continue;
      hit->Print();

      auto ntupleid = rmg_man->GetNtupleID(hit->detector_uid);

      int col_id = 0;
      ana_man->FillNtupleIColumn(ntupleid, col_id++, event->GetEventID());
      if (!fNtuplePerDetector) {
        ana_man->FillNtupleIColumn(ntupleid, col_id++, hit->detector_uid);
      }
      ana_man->FillNtupleIColumn(ntupleid, col_id++, hit->particle_type);

      // store track IDs if instructed
      if (fStoreTrackID) {
        ana_man->FillNtupleIColumn(ntupleid, col_id++, hit->track_id);
        ana_man->FillNtupleIColumn(ntupleid, col_id++, hit->parent_track_id);
      }

      FillNtupleFOrDColumn(
          ana_man,
          ntupleid,
          col_id++,
          hit->energy_deposition / u::keV,
          fStoreSinglePrecisionEnergy
      );
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_time / u::ns);


      // extract position based on position mode and hit
      G4ThreeVector position = RMGOutputTools::get_position(hit, fPositionMode);

      FillNtupleFOrDColumn(
          ana_man,
          ntupleid,
          col_id++,
          position.getX() / u::m,
          fStoreSinglePrecisionPosition
      );
      FillNtupleFOrDColumn(
          ana_man,
          ntupleid,
          col_id++,
          position.getY() / u::m,
          fStoreSinglePrecisionPosition
      );
      FillNtupleFOrDColumn(
          ana_man,
          ntupleid,
          col_id++,
          position.getZ() / u::m,
          fStoreSinglePrecisionPosition
      );

      // save also the other points if requested
      if (fPositionMode == RMGOutputTools::PositionMode::kBoth) {

        // save post-step
        position = hit->global_position_prestep;
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            position.getX() / u::m,
            fStoreSinglePrecisionPosition
        );
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            position.getY() / u::m,
            fStoreSinglePrecisionPosition
        );
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            position.getZ() / u::m,
            fStoreSinglePrecisionPosition
        );

        // save avg
        position = hit->global_position_poststep;
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            position.getX() / u::m,
            fStoreSinglePrecisionPosition
        );
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            position.getY() / u::m,
            fStoreSinglePrecisionPosition
        );
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            position.getZ() / u::m,
            fStoreSinglePrecisionPosition
        );
      }

      if (fStoreVelocity) {
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            hit->velocity_pre / u::m * u::ns,
            fStoreSinglePrecisionPosition
        );
        FillNtupleFOrDColumn(
            ana_man,
            ntupleid,
            col_id++,
            hit->velocity_post / u::m * u::ns,
            fStoreSinglePrecisionPosition
        );
      }
      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(ntupleid);
    }
  }
}
void RMGScintillatorOutputScheme::SetPositionModeString(std::string mode) {

  try {
    this->SetPositionMode(RMGTools::ToEnum<RMGOutputTools::PositionMode>(mode, "position mode"));
  } catch (const std::bad_cast&) { return; }
}

void RMGScintillatorOutputScheme::DefineCommands() {

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(
          this,
          "/RMG/Output/Scintillator/",
          "Commands for controlling output from hits in scintillator detectors."
      )
  );

  fMessengers.back()
      ->DeclareMethodWithUnit("EdepCutLow", "keV", &RMGScintillatorOutputScheme::SetEdepCutLow)
      .SetGuidance("Set a lower energy cut that has to be met for this event to be stored.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("EdepCutHigh", "keV", &RMGScintillatorOutputScheme::SetEdepCutHigh)
      .SetGuidance("Set an upper energy cut that has to be met for this event to be stored.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethod("AddDetectorForEdepThreshold", &RMGScintillatorOutputScheme::AddEdepCutDetector)
      .SetGuidance("Take this detector into account for the filtering by /EdepThreshold.")
      .SetParameterName("det_uid", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareProperty("DiscardZeroEnergyHits", fDiscardZeroEnergyHits)
      .SetGuidance("Discard hits with zero energy.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareProperty("StoreParticleVelocities", fStoreVelocity)
      .SetGuidance("Store velocities of particle in the output file.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareProperty("StoreTrackID", fStoreTrackID)
      .SetGuidance("Store Track IDs for hits in the output file.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareProperty("StoreSinglePrecisionPosition", fStoreSinglePrecisionPosition)
      .SetGuidance("Use float32 (instead of float64) for position output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareProperty("StoreSinglePrecisionEnergy", fStoreSinglePrecisionEnergy)
      .SetGuidance("Use float32 (instead of float64) for energy output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);


  fMessengers.back()
      ->DeclareMethod("StepPositionMode", &RMGScintillatorOutputScheme::SetPositionModeString)
      .SetGuidance("Select which position of the step to store")
      .SetParameterName("mode", false)
      .SetCandidates(RMGTools::GetCandidates<RMGOutputTools::PositionMode>())
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);

  // clustering pars

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(
          this,
          "/RMG/Output/Scintillator/Cluster/",
          "Commands for controlling clustering of hits in scintillator detectors."
      )
  );

  fMessengers.back()
      ->DeclareProperty("PreClusterOutputs", fPreClusterHits)
      .SetGuidance("Pre-Cluster output hits before saving")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareProperty("CombineLowEnergyElectronTracks", fPreClusterPars.combine_low_energy_tracks)
      .SetGuidance("Merge low energy electron tracks.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareProperty("RedistributeGammaEnergy", fPreClusterPars.reassign_gamma_energy)
      .SetGuidance("Redistribute energy deposited by gamma tracks to nearby electron tracks.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("PreClusterDistance", "um", &RMGScintillatorOutputScheme::SetClusterDistance)
      .SetGuidance("Set a distance threshold for the bulk pre-clustering.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit(
          "PreClusterTimeThreshold",
          "us",
          &RMGScintillatorOutputScheme::SetClusterTimeThreshold
      )
      .SetGuidance("Set a time threshold for  pre-clustering.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit(
          "ElectronTrackEnergyThreshold",
          "keV",
          &RMGScintillatorOutputScheme::SetElectronTrackEnergyThreshold
      )
      .SetGuidance("Set a energy threshold for tracks to be merged.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

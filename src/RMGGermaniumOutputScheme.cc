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

#include "RMGGermaniumOutputScheme.hh"

#include <set>

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCtable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SDManager.hh"

#include "RMGGermaniumDetector.hh"
#include "RMGHardware.hh"
#include "RMGIpc.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOutputTools.hh"
#include "RMGTools.hh"

namespace u = CLHEP;

RMGGermaniumOutputScheme::RMGGermaniumOutputScheme() {

  // set default clustering parameters
  fPreClusterPars.cluster_time_threshold = 10 * u::us;
  fPreClusterPars.cluster_distance = 10 * u::um;
  fPreClusterPars.cluster_distance_surface = 1 * u::um;
  fPreClusterPars.surface_thickness = 2 * u::mm;
  fPreClusterPars.track_energy_threshold = 10 * u::keV;
  fPreClusterPars.combine_low_energy_tracks = false;
  fPreClusterPars.reassign_gamma_energy = false;

  this->DefineCommands();
}


void RMGGermaniumOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {

  auto rmg_man = RMGManager::Instance();
  const auto det_cons = rmg_man->GetDetectorConstruction();
  const auto detectors = det_cons->GetDetectorMetadataMap();

  std::set<int> registered_uids;
  std::map<std::string, int> registered_ntuples;
  for (auto&& det : detectors) {
    if (det.second.type != RMGDetectorType::kGermanium) continue;

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

    auto id =
        rmg_man->RegisterNtuple(det.second.uid, ana_man->CreateNtuple(ntuple_name, "Event data"));
    registered_ntuples.emplace(ntuple_name, id);
    RMGIpc::SendIpcNonBlocking(
        RMGIpc::CreateMessage("output_table", std::string("germanium\x1e") + ntuple_name));

    // store the indices
    ana_man->CreateNtupleIColumn(id, "evtid");
    if (!fNtuplePerDetector) { ana_man->CreateNtupleIColumn(id, "det_uid"); }
    ana_man->CreateNtupleIColumn(id, "particle");

    // also store track IDs if instructed
    if (fStoreTrackID) {
      ana_man->CreateNtupleIColumn(id, "trackid");
      ana_man->CreateNtupleIColumn(id, "parent_trackid");
    }
    // store the floating points values
    CreateNtupleFOrDColumn(ana_man, id, "edep_in_keV", fStoreSinglePrecisionEnergy);
    ana_man->CreateNtupleDColumn(id, "time_in_ns");
    CreateNtupleFOrDColumn(ana_man, id, "xloc_in_m", fStoreSinglePrecisionPosition);
    CreateNtupleFOrDColumn(ana_man, id, "yloc_in_m", fStoreSinglePrecisionPosition);
    CreateNtupleFOrDColumn(ana_man, id, "zloc_in_m", fStoreSinglePrecisionPosition);
    CreateNtupleFOrDColumn(ana_man, id, "dist_to_surf_in_m", fStoreSinglePrecisionPosition);

    // save also a second position if requested
    if (fPositionMode == RMGOutputTools::PositionMode::kBoth) {

      CreateNtupleFOrDColumn(ana_man, id, "xloc_pre_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "yloc_pre_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "zloc_pre_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "dist_to_surf_pre_in_m", fStoreSinglePrecisionPosition);

      CreateNtupleFOrDColumn(ana_man, id, "xloc_post_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "yloc_post_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "zloc_post_in_m", fStoreSinglePrecisionPosition);
      CreateNtupleFOrDColumn(ana_man, id, "dist_to_surf_post_in_m", fStoreSinglePrecisionPosition);
    }
    ana_man->FinishNtuple(id);
  }
}

RMGDetectorHitsCollection* RMGGermaniumOutputScheme::GetHitColl(const G4Event* event) {
  auto sd_man = G4SDManager::GetSDMpointer();

  auto hit_coll_id = sd_man->GetCollectionID("Germanium/Hits");
  if (hit_coll_id < 0) {
    RMGLog::OutDev(RMGLog::error, "Could not find hit collection Germanium/Hits");
    return nullptr;
  }

  auto hit_coll =
      dynamic_cast<RMGDetectorHitsCollection*>(event->GetHCofThisEvent()->GetHC(hit_coll_id));

  if (!hit_coll) {
    RMGLog::Out(RMGLog::error, "Could not find hit collection associated with event");
    return nullptr;
  }

  return hit_coll;
}


bool RMGGermaniumOutputScheme::ShouldDiscardEvent(const G4Event* event) {

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
    RMGLog::Out(RMGLog::debug, "Discarding event - energy threshold has not been met", event_edep,
        fEdepCutLow, fEdepCutHigh);
    return true;
  }

  return false;
}


void RMGGermaniumOutputScheme::StoreEvent(const G4Event* event) {

  // get the hit collection - with preclustering if requested
  auto hit_coll = GetHitColl(event);

  if (!hit_coll) return;

  if (hit_coll->entries() <= 0) {
    RMGLog::OutDev(RMGLog::debug, "Hit collection is empty");
    return;
  } else {
    RMGLog::OutDev(RMGLog::debug, "Hit collection contains ", hit_coll->entries(), " hits");
  }

  if (fPreClusterHits)
    hit_coll = RMGOutputTools::pre_cluster_hits(hit_coll, fPreClusterPars, true, false);


  auto rmg_man = RMGManager::Instance();
  if (rmg_man->IsPersistencyEnabled()) {
    RMGLog::OutDev(RMGLog::debug, "Filling persistent data vectors");
    const auto ana_man = G4AnalysisManager::Instance();

    for (auto hit : *hit_coll->GetVector()) {

      if (!hit or (hit->energy_deposition == 0 and this->fDiscardZeroEnergyHits)) continue;

      hit->Print();
      auto ntupleid = rmg_man->GetNtupleID(hit->detector_uid);

      int col_id = 0;
      // store the indices
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

      FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, hit->energy_deposition / u::keV,
          fStoreSinglePrecisionEnergy);
      ana_man->FillNtupleDColumn(ntupleid, col_id++, hit->global_time / u::ns);

      // get the position and distance to save
      G4ThreeVector position = RMGOutputTools::get_position(hit, fPositionMode);
      double distance = RMGOutputTools::get_distance(hit, fPositionMode);

      FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getX() / u::m,
          fStoreSinglePrecisionPosition);
      FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getY() / u::m,
          fStoreSinglePrecisionPosition);
      FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getZ() / u::m,
          fStoreSinglePrecisionPosition);
      FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, distance / u::m,
          fStoreSinglePrecisionPosition);

      // save also post-step if requested
      if (fPositionMode == RMGOutputTools::PositionMode::kBoth) {

        // save post-step
        position = hit->global_position_prestep;
        distance = hit->distance_to_surface_prestep;
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getX() / u::m,
            fStoreSinglePrecisionPosition);
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getY() / u::m,
            fStoreSinglePrecisionPosition);
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getZ() / u::m,
            fStoreSinglePrecisionPosition);
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, distance / u::m,
            fStoreSinglePrecisionPosition);

        // save avg
        position = hit->global_position_poststep;
        distance = hit->distance_to_surface_poststep;
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getX() / u::m,
            fStoreSinglePrecisionPosition);
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getY() / u::m,
            fStoreSinglePrecisionPosition);
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, position.getZ() / u::m,
            fStoreSinglePrecisionPosition);
        FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, distance / u::m,
            fStoreSinglePrecisionPosition);
      }

      // NOTE: must be called here for hit-oriented output
      ana_man->AddNtupleRow(ntupleid);
    }
  }

  // if needed we should delete the collection we own
  if (fPreClusterHits) delete hit_coll;
}

std::optional<G4ClassificationOfNewTrack> RMGGermaniumOutputScheme::StackingActionClassify(const G4Track* aTrack,
    int stage) {
  // we are only interested in stacking optical photons into stage 1 after stage 0 finished.
  if (stage != 0) return std::nullopt;

  // defer tracking of optical photons.
  if (fDiscardPhotonsIfNoGermaniumEdep &&
      aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition())
    return fWaiting;
  return std::nullopt;
}

std::optional<bool> RMGGermaniumOutputScheme::StackingActionNewStage(const int stage) {
  // we are only interested in stacking optical photons into stage 1 after stage 0 finished.
  if (stage != 0) return std::nullopt;
  // if we do not want to discard any photons ourselves, let other output schemes decide (i.e. not
  // force `true` on them).
  if (!fDiscardPhotonsIfNoGermaniumEdep) return std::nullopt;

  const auto event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
  // discard all waiting events, if there was no energy deposition in Germanium.
  return ShouldDiscardEvent(event) ? std::make_optional(false) : std::nullopt;
}

void RMGGermaniumOutputScheme::SetPositionModeString(std::string mode) {

  try {
    this->SetPositionMode(RMGTools::ToEnum<RMGOutputTools::PositionMode>(mode, "position mode"));
  } catch (const std::bad_cast&) { return; }
}
void RMGGermaniumOutputScheme::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Output/Germanium/",
      "Commands for controlling output from hits in germanium detectors.");

  fMessenger->DeclareMethodWithUnit("EdepCutLow", "keV", &RMGGermaniumOutputScheme::SetEdepCutLow)
      .SetGuidance("Set a lower energy cut that has to be met for this event to be stored.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareMethodWithUnit("EdepCutHigh", "keV", &RMGGermaniumOutputScheme::SetEdepCutHigh)
      .SetGuidance("Set an upper energy cut that has to be met for this event to be stored.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("PreClusterOutputs", fPreClusterHits)
      .SetGuidance("Pre-Cluster output hits before saving")
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareProperty("CombineLowEnergyElectronTracks", fPreClusterPars.combine_low_energy_tracks)
      .SetGuidance("Merge low energy electron tracks.")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("RedistributeGammaEnergy", fPreClusterPars.reassign_gamma_energy)
      .SetGuidance("Redistribute energy deposited by gamma tracks to nearby electron tracks.")
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit("PreClusterDistance", "um",
          &RMGGermaniumOutputScheme::SetClusterDistance)
      .SetGuidance("Set a distance threshold for the bulk pre-clustering.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);
  fMessenger
      ->DeclareMethodWithUnit("PreClusterDistanceSurface", "um",
          &RMGGermaniumOutputScheme::SetClusterDistanceSurface)
      .SetGuidance("Set a distance threshold for the surface pre-clustering.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit("PreClusterTimeThreshold", "us",
          &RMGGermaniumOutputScheme::SetClusterTimeThreshold)
      .SetGuidance("Set a time threshold for  pre-clustering.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit("SurfaceThickness", "mm",
          &RMGGermaniumOutputScheme::SetSurfaceThickness)
      .SetGuidance("Set a surface thickness for the Germanium detector.")
      .SetParameterName("thickness", false)
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethodWithUnit("ElectronTrackEnergyThreshold", "keV",
          &RMGGermaniumOutputScheme::SetElectronTrackEnergyThreshold)
      .SetGuidance("Set a energy threshold for tracks to be merged.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fMessenger
      ->DeclareMethod("AddDetectorForEdepThreshold", &RMGGermaniumOutputScheme::AddEdepCutDetector)
      .SetGuidance("Take this detector into account for the filtering by /EdepThreshold.")
      .SetParameterName("det_uid", false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("DiscardPhotonsIfNoGermaniumEdep", fDiscardPhotonsIfNoGermaniumEdep)
      .SetGuidance("Discard optical photons (before simulating them), if no edep in germanium "
                   "detectors occurred in the same event.")
      .SetGuidance("note: If another output scheme also requests the photons to be discarded, the "
                   "germanium edep filter does not force the photons to be simulated.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionPosition", fStoreSinglePrecisionPosition)
      .SetGuidance("Use float32 (instead of float64) for position output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionEnergy", fStoreSinglePrecisionEnergy)
      .SetGuidance("Use float32 (instead of float64) for energy output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("DiscardZeroEnergyHits", fDiscardZeroEnergyHits)
      .SetGuidance("Discard hits with zero energy.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreTrackID", fStoreTrackID)
      .SetGuidance("Store Track IDs for hits in the output file.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("StepPositionMode", &RMGGermaniumOutputScheme::SetPositionModeString)
      .SetGuidance("Select which position of the step to store")
      .SetParameterName("mode", false)
      .SetCandidates(RMGTools::GetCandidates<RMGOutputTools::PositionMode>())
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);
}

// vim: tabstop=2 shiftwidth=2 expandtab

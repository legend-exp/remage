// Copyright (C) 2025 Toby Dixon <https://orcid.org/0000-0001-8787-6336>
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

#include "RMGOutputTools.hh"

#include <memory>
#include <unordered_map>

#include "G4AffineTransform.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4LogicalVolume.hh"
#include "G4MultiUnion.hh"
#include "G4TransportationManager.hh"
#include "G4VSolid.hh"

#include "RMGDetectorHit.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#include "magic_enum/magic_enum.hpp"


namespace {
  // Configuration for bounding sphere optimization
  bool g_filter_germanium_only = false;
  
  // Cache structure for volume geometry data
  struct VolumeCache {
    G4AffineTransform inverse_transform;
    const G4VSolid* solid;
    size_t num_daughters;
    std::vector<G4AffineTransform> daughter_transforms;
    std::vector<const G4VSolid*> daughter_solids;
    std::vector<bool> daughter_is_multiunion;
    std::vector<G4ThreeVector> daughter_centers;  // bounding sphere centers in parent local coords
    std::vector<double> daughter_radii;           // bounding sphere radii
    std::vector<bool> daughter_is_germanium;      // whether daughter is registered as Germanium detector
  };
  
  // Cache for volume data, keyed by physical volume pointer
  std::unordered_map<const G4VPhysicalVolume*, VolumeCache> volume_cache;
}

G4ThreeVector RMGOutputTools::get_position(RMGDetectorHit* hit, RMGOutputTools::PositionMode mode) {
  G4ThreeVector position;

  // all gamma interactions are discrete at the post-step
  if (mode == RMGOutputTools::PositionMode::kPostStep or
      hit->particle_type == G4Gamma::GammaDefinition()->GetPDGEncoding()) {
    position = hit->global_position_poststep;
  } else if (mode == RMGOutputTools::PositionMode::kPreStep) {
    position = hit->global_position_prestep;
  } else if (mode == RMGOutputTools::PositionMode::kAverage or
             mode == RMGOutputTools::PositionMode::kBoth) {

    position = hit->global_position_average;
  } else
    RMGLog::Out(
        RMGLog::fatal,
        "fPositionMode is not set to kPreStep, kPostStep or kAverage instead ",
        magic_enum::enum_name<RMGOutputTools::PositionMode>(mode)
    );

  return position;
}

double RMGOutputTools::get_distance(RMGDetectorHit* hit, RMGOutputTools::PositionMode mode) {
  double distance = 0;

  // all gamma interactions are discrete at the post-step
  if (mode == RMGOutputTools::PositionMode::kPostStep or
      hit->particle_type == G4Gamma::GammaDefinition()->GetPDGEncoding()) {
    distance = hit->distance_to_surface_poststep;
  } else if (mode == RMGOutputTools::PositionMode::kPreStep) {
    distance = hit->distance_to_surface_prestep;
  } else if (mode == RMGOutputTools::PositionMode::kAverage or
             mode == RMGOutputTools::PositionMode::kBoth) {

    distance = hit->distance_to_surface_average;
  } else
    RMGLog::Out(
        RMGLog::fatal,
        "fPositionMode is not set to kPreStep, kPostStep or kAverage instead ",
        magic_enum::enum_name<RMGOutputTools::PositionMode>(mode)
    );

  return distance;
}

void RMGOutputTools::SetFilterGermaniumOnly(bool enable) {
  g_filter_germanium_only = enable;
  // Clear cache when filter setting changes to rebuild with correct detector status
  volume_cache.clear();
}

RMGDetectorHit* RMGOutputTools::average_hits(
    std::vector<RMGDetectorHit*> hits,
    bool compute_distance_to_surface,
    bool compute_velocity
) {

  if (hits.empty()) {
    RMGLog::OutDev(RMGLog::error, "Cannot average empty set of hits");
    return nullptr;
  }
  auto hit = new RMGDetectorHit();

  hit->energy_deposition = 0;
  for (auto hit_tmp : hits) hit->energy_deposition += hit_tmp->energy_deposition;

  // by construction the particle type, detuid and track id should all be the same
  hit->detector_uid = hits.front()->detector_uid;
  hit->particle_type = hits.front()->particle_type;
  hit->track_id = hits.front()->track_id;
  hit->parent_track_id = hits.front()->parent_track_id;

  // all physical volumes should be the same
  hit->physical_volume = hits.back()->physical_volume;

  // time from first hit
  hit->global_time = hits.front()->global_time;

  // The cluster represents a list of consecutive steps, so we can
  // take the pre and post step from the first and last hit in the cluster.
  hit->global_position_prestep = hits.front()->global_position_prestep;
  hit->global_position_poststep = hits.back()->global_position_poststep;

  // issue: this could be outside the volume!
  hit->global_position_average = (hit->global_position_prestep + hit->global_position_poststep) / 2.;

  // check if the average point is inside
  auto navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  if (navigator->LocateGlobalPointAndSetup(hit->global_position_average) != hit->physical_volume)
    return nullptr;


  // compute the distance to the surface, for the pre/post step this is already done
  // but a new calculation is needed for the average.
  if (compute_distance_to_surface) {
    hit->distance_to_surface_prestep = hits.front()->distance_to_surface_prestep;
    hit->distance_to_surface_poststep = hits.back()->distance_to_surface_poststep;
    hit->distance_to_surface_average = distance_to_surface(
        hits.back()->physical_volume,
        hit->global_position_average
    );
  }


  // prestep velocity from the first hit and poststep from the last
  if (compute_velocity) {
    hit->velocity_pre = hits.front()->velocity_pre;
    hit->velocity_post = hits.back()->velocity_post;
  }

  return hit;
}

bool RMGOutputTools::check_step_point_containment(
    const G4StepPoint* step_point,
    RMGDetectorType det_type
) {

  const auto pv = step_point->GetTouchableHandle()->GetVolume();
  auto pv_name = pv->GetName();
  const auto pv_copynr = step_point->GetTouchableHandle()->GetCopyNumber();

  // check if physical volume is registered as germanium detector
  const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  try {
    auto d_type = det_cons->GetDetectorMetadata({pv_name, pv_copynr}).type;
    if (d_type != det_type) {
      RMGLog::OutFormatDev(
          RMGLog::debug_event,
          "Volume '{}' (copy nr. {} not registered as {} detector",
          pv_name,
          pv_copynr,
          magic_enum::enum_name<RMGDetectorType>(det_type)
      );
      return false;
    }
  } catch (const std::out_of_range& e) {
    RMGLog::OutFormatDev(
        RMGLog::debug_event,
        "Volume '{}' (copy nr. {}) not registered as detector",
        pv_name,
        pv_copynr
    );
    return false;
  }
  return true;
}

void RMGOutputTools::redistribute_gamma_energy(
    std::map<int, std::vector<RMGDetectorHit*>> hits_map,
    RMGOutputTools::ClusterPars cluster_pars,
    bool has_distance_to_surface
) {

  RMGLog::Out(RMGLog::debug_event, "Merging gamma tracks ");

  // for tracks of gammas look for a step close to each post-step point
  // to redistribute the energy to
  for (const auto& [trackid, input_hits] : hits_map) {

    // only apply to gamma
    if (input_hits.front()->particle_type != G4Gamma::GammaDefinition()->GetPDGEncoding()) continue;

    // loop through the track
    for (auto hit : input_hits) {

      // only focus on hits with an energy deposit
      if (hit->energy_deposition == 0) continue;

      // extract a threshold
      double threshold = (not has_distance_to_surface) or ((hit->distance_to_surface_prestep) >
                                                           cluster_pars.surface_thickness)
                             ? cluster_pars.cluster_distance
                             : cluster_pars.cluster_distance_surface;


      // loop over secondary tracks
      for (const auto& [second_trackid, second_input_hits] : hits_map) {
        if (second_trackid == trackid) continue;

        // compute distance between the gamma post-step and the first hit of the other track and
        // that of the other track.
        if ((hit->global_position_poststep - second_input_hits.front()->global_position_prestep).mag() <
            threshold) {
          // give this hit the energy deposition
          second_input_hits.front()->energy_deposition += hit->energy_deposition;
          hit->energy_deposition = 0;
        }
      }
    }
  }
}


std::map<int, std::vector<RMGDetectorHit*>> RMGOutputTools::combine_low_energy_tracks(
    std::map<int, std::vector<RMGDetectorHit*>> hits_map,
    RMGOutputTools::ClusterPars cluster_pars,
    bool has_distance_to_surface
) {

  RMGLog::Out(RMGLog::debug_event, "Merging low energy electron tracks ");


  // for tracks below an energy threshold look for a close neighbour to merge it with
  // only done for e-.
  std::map<int, std::vector<RMGDetectorHit*>> output_hits = hits_map;

  for (const auto& [trackid, input_hits] : hits_map) {

    // only apply to e-
    if (input_hits.front()->particle_type != G4Electron::ElectronDefinition()->GetPDGEncoding())
      continue;

    // compute energy of each track
    double energy = 0;
    for (auto hit : input_hits) { energy += hit->energy_deposition; }

    // continue for high energy tracks
    if (energy > cluster_pars.track_energy_threshold) continue;

    // distance threshold to merge tracks
    double threshold = (not has_distance_to_surface) or
                               ((input_hits.front()->distance_to_surface_prestep) >
                                cluster_pars.surface_thickness)
                           ? cluster_pars.cluster_distance
                           : cluster_pars.cluster_distance_surface;

    // now search for another track to merge it with
    int cluster_trackid = -1;
    double cluster_energy = 0;

    // loop over secondary tracks
    for (const auto& [second_trackid, second_input_hits] : hits_map) {
      if (second_trackid == trackid) continue;

      // only cluster into high energy tracks
      energy = 0;
      for (auto hit : second_input_hits) { energy += hit->energy_deposition; }


      // compute distance between the first step of this track and that of the
      // other track.
      if (energy > cluster_energy && (input_hits.front()->global_position_prestep -
                                      second_input_hits.front()->global_position_prestep)
                                             .mag() < threshold) {

        cluster_energy = energy;
        cluster_trackid = second_trackid;
      }
    }

    if (cluster_trackid != -1) {

      // change all the track-ids
      for (auto hit : output_hits[trackid]) { hit->track_id = cluster_trackid; }

      // add these elements to the start of the second track
      output_hits[cluster_trackid].insert(
          output_hits[cluster_trackid].begin(),
          output_hits[trackid].begin(),
          output_hits[trackid].end()
      );

      output_hits.erase(trackid);
      RMGLog::Out(RMGLog::debug_event, "Removing trackid ", trackid);
    }
  }
  return output_hits;
}

std::shared_ptr<RMGDetectorHitsCollection> RMGOutputTools::pre_cluster_hits(
    const RMGDetectorHitsCollection* hits,
    RMGOutputTools::ClusterPars cluster_pars,
    bool has_distance_to_surface,
    bool has_velocity
) {

  // organise hits into a map based on trackid
  std::map<int, std::vector<RMGDetectorHit*>> hits_map;

  for (auto hit : *hits->GetVector()) hits_map[hit->track_id].push_back(hit);

  // if requested we can combine low energy tracks to reduce further file size
  if (cluster_pars.combine_low_energy_tracks)
    hits_map = combine_low_energy_tracks(hits_map, cluster_pars, has_distance_to_surface);

  if (cluster_pars.reassign_gamma_energy)
    redistribute_gamma_energy(hits_map, cluster_pars, has_distance_to_surface);

  // create a vector of clusters of hits
  std::vector<std::vector<RMGDetectorHit*>> hits_vector;

  // keep track of the current cluster
  // loop over trackid and then hits in each track
  for (const auto& [trackid, input_hits] : hits_map) {
    RMGDetectorHit* cluster_first_hit = nullptr;

    for (auto hit : input_hits) {

      if (!hit) continue;

      // within track clustering

      // if the hit is:
      // - the first in a track
      // - in a new detector (compared to the current cluster)
      // - more than the time-threshold since the first hit of the cluster
      // then we need to start a new cluster.

      bool start_new_cluster = (cluster_first_hit == nullptr) or
                               (hit->track_id != cluster_first_hit->track_id) or
                               (hit->detector_uid != cluster_first_hit->detector_uid) or
                               (std::abs(hit->global_time - cluster_first_hit->global_time) >
                                cluster_pars.cluster_time_threshold);
      // check distances and if the track moved from surface to bulk
      if (!start_new_cluster) {
        bool is_surface = has_distance_to_surface and
                          (hit->distance_to_surface_average < cluster_pars.surface_thickness);
        bool is_surface_first_hit = has_distance_to_surface and
                                    (cluster_first_hit->distance_to_surface_average <
                                     cluster_pars.surface_thickness);

        // start a new cluster if the previous step was in the surface and the new is in the bulk
        bool surface_transition = (is_surface != is_surface_first_hit);

        // get the right distance to pre-cluster
        double threshold = is_surface ? cluster_pars.cluster_distance_surface
                                      : cluster_pars.cluster_distance;


        // start a new cluster also if the distance is above the threshold
        start_new_cluster = surface_transition || (hit->global_position_average -
                                                   cluster_first_hit->global_position_average)
                                                          .mag() >= threshold;
      }

      // add the hit to the correct vector
      if (start_new_cluster) {
        hits_vector.emplace_back();
        hits_vector.back().push_back(hit);

        cluster_first_hit = hit;
      } else {
        hits_vector.back().push_back(hit);
      }
    }
  }

  // create a container for the output hits
  auto out = std::make_shared<RMGDetectorHitsCollection>();

  // average the hits
  for (const auto& value : hits_vector) {

    // average the hit and insert into the collection
    auto averaged_hit = average_hits(value, has_distance_to_surface, has_velocity);
    if (averaged_hit) out->insert(averaged_hit);
    else {

      for (auto hit : value) {
        hit->Print();
        out->insert(new RMGDetectorHit(*hit));
      }
    }
  }

  return out;
}


double RMGOutputTools::distance_to_surface(const G4VPhysicalVolume* pv, const G4ThreeVector& position) {

  // Check cache first
  auto cache_it = volume_cache.find(pv);
  if (cache_it == volume_cache.end()) {
    // Not in cache, build cache entry
    VolumeCache cache;
    const auto lv = pv->GetLogicalVolume();
    
    // Cache parent transform and solid
    cache.inverse_transform = G4AffineTransform(pv->GetRotation(), pv->GetTranslation()).Inverse();
    cache.solid = lv->GetSolid();
    cache.num_daughters = lv->GetNoDaughters();
    
    // Cache daughter data
    cache.daughter_transforms.reserve(cache.num_daughters);
    cache.daughter_solids.reserve(cache.num_daughters);
    cache.daughter_is_multiunion.reserve(cache.num_daughters);
    cache.daughter_centers.reserve(cache.num_daughters);
    cache.daughter_radii.reserve(cache.num_daughters);
    cache.daughter_is_germanium.reserve(cache.num_daughters);
    
    
    for (size_t i = 0; i < cache.num_daughters; ++i) {
      const auto daughter = lv->GetDaughter(i);
      cache.daughter_transforms.emplace_back(
        G4AffineTransform(daughter->GetRotation(), daughter->GetTranslation()).Inverse()
      );
      const auto daughter_solid = daughter->GetLogicalVolume()->GetSolid();
      cache.daughter_solids.push_back(daughter_solid);
      // Do type check once during cache creation
      cache.daughter_is_multiunion.push_back(
        daughter_solid->GetEntityType() == "G4MultiUnion"
      );
      
      // Compute bounding sphere for early rejection
      G4ThreeVector pMin, pMax;
      daughter_solid->BoundingLimits(pMin, pMax);
      G4ThreeVector local_center = (pMin + pMax) * 0.5;
      double local_radius = (pMax - local_center).mag();
      
      // Transform center to parent coordinates (daughter translation + rotation of local center)
      const auto daughter_rot = daughter->GetRotation();
      G4ThreeVector parent_center = daughter->GetTranslation();
      if (daughter_rot) {
        parent_center += (*daughter_rot) * local_center;
      } else {
        parent_center += local_center;
      }
      
      cache.daughter_centers.push_back(parent_center);
      cache.daughter_radii.push_back(local_radius);
    }
    
    cache_it = volume_cache.emplace(pv, std::move(cache)).first;
  }
  
  const auto& cache = cache_it->second;
  
  // Transform to local coordinates and get distance to parent surface
  const G4ThreeVector local_pos = cache.inverse_transform.TransformPoint(position);
  double dist = cache.solid->DistanceToOut(local_pos);

  // Check distance to daughters
  for (size_t i = 0; i < cache.num_daughters; ++i) {    
    // Early rejection: check distance to bounding sphere first (cheap)
    const double center_dist = (cache.daughter_centers[i] - local_pos).mag();
    const double sphere_surface_dist = center_dist - cache.daughter_radii[i];
    
    // Skip if bounding sphere is farther than current best distance
    if (sphere_surface_dist > dist) continue;
    
    // Only do expensive surface calculation if bounding sphere is close
    const G4ThreeVector sample_point = cache.daughter_transforms[i].TransformPoint(local_pos);
    
    // Handle MultiUnion flag
    if (cache.daughter_is_multiunion[i]) {
      auto mu = const_cast<G4MultiUnion*>(static_cast<const G4MultiUnion*>(cache.daughter_solids[i]));
      mu->SetAccurateSafety(true);
      const double sample_dist = cache.daughter_solids[i]->DistanceToIn(sample_point);
      if (sample_dist < dist) dist = sample_dist;
      mu->SetAccurateSafety(false);
    } else {
      const double sample_dist = cache.daughter_solids[i]->DistanceToIn(sample_point);
      if (sample_dist < dist) dist = sample_dist;
    }
  }
  
  return dist;
}

bool RMGOutputTools::is_within_surface_safety(const G4VPhysicalVolume* pv, const G4ThreeVector& position, double safety) {

  // Check cache first
  auto cache_it = volume_cache.find(pv);
  if (cache_it == volume_cache.end()) {
    // Not in cache, build cache entry (same as distance_to_surface)
    VolumeCache cache;
    const auto lv = pv->GetLogicalVolume();
    
    cache.inverse_transform = G4AffineTransform(pv->GetRotation(), pv->GetTranslation()).Inverse();
    cache.solid = lv->GetSolid();
    cache.num_daughters = lv->GetNoDaughters();
    
    cache.daughter_transforms.reserve(cache.num_daughters);
    cache.daughter_solids.reserve(cache.num_daughters);
    cache.daughter_is_multiunion.reserve(cache.num_daughters);
    cache.daughter_centers.reserve(cache.num_daughters);
    cache.daughter_radii.reserve(cache.num_daughters);
    cache.daughter_is_germanium.reserve(cache.num_daughters);
    
    // Get detector construction for Germanium filtering
    const auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
    
    for (size_t i = 0; i < cache.num_daughters; ++i) {
      const auto daughter = lv->GetDaughter(i);
      cache.daughter_transforms.emplace_back(
        G4AffineTransform(daughter->GetRotation(), daughter->GetTranslation()).Inverse()
      );
      const auto daughter_solid = daughter->GetLogicalVolume()->GetSolid();
      cache.daughter_solids.push_back(daughter_solid);
      cache.daughter_is_multiunion.push_back(
        daughter_solid->GetEntityType() == "G4MultiUnion"
      );
      
      // Check if daughter is a Germanium detector
      bool is_germanium = false;
      if (g_filter_germanium_only) {
        try {
          auto d_type = det_cons->GetDetectorMetadata(
            {daughter->GetName(), daughter->GetCopyNo()}
          ).type;
          is_germanium = (d_type == RMGDetectorType::kGermanium);
        } catch (const std::out_of_range&) {
          is_germanium = false;
        }
      }
      cache.daughter_is_germanium.push_back(is_germanium);
      
      // Compute bounding sphere for early rejection
      G4ThreeVector pMin, pMax;
      daughter_solid->BoundingLimits(pMin, pMax);
      G4ThreeVector local_center = (pMin + pMax) * 0.5;
      double local_radius = (pMax - local_center).mag();
      
      // Transform center to parent coordinates
      const auto daughter_rot = daughter->GetRotation();
      G4ThreeVector parent_center = daughter->GetTranslation();
      if (daughter_rot) {
        parent_center += (*daughter_rot) * local_center;
      } else {
        parent_center += local_center;
      }
      
      cache.daughter_centers.push_back(parent_center);
      cache.daughter_radii.push_back(local_radius);
    }
    
    cache_it = volume_cache.emplace(pv, std::move(cache)).first;
  }
  
  const auto& cache = cache_it->second;
  
  // Transform to local coordinates
  const G4ThreeVector local_pos = cache.inverse_transform.TransformPoint(position);
  
  // Check parent surface - early exit if within safety
  if (cache.solid->DistanceToOut(local_pos) < safety) return true;

  // Check daughters - early exit as soon as we find one within safety
  for (size_t i = 0; i < cache.num_daughters; ++i) {
    // Skip non-Germanium daughters if filtering is enabled
    if (g_filter_germanium_only && !cache.daughter_is_germanium[i]) continue;
    
    // Early rejection: check distance to bounding sphere first (cheap)
    const double center_dist = (cache.daughter_centers[i] - local_pos).mag();
    const double sphere_surface_dist = center_dist - cache.daughter_radii[i];
    
    // Skip if bounding sphere is farther than safety threshold
    if (sphere_surface_dist > safety) continue;
    
    // Only do expensive surface calculation if bounding sphere is close
    const G4ThreeVector sample_point = cache.daughter_transforms[i].TransformPoint(local_pos);
    
    double sample_dist;
    if (cache.daughter_is_multiunion[i]) {
      auto mu = const_cast<G4MultiUnion*>(static_cast<const G4MultiUnion*>(cache.daughter_solids[i]));
      mu->SetAccurateSafety(true);
      sample_dist = cache.daughter_solids[i]->DistanceToIn(sample_point);
      mu->SetAccurateSafety(false);
    } else {
      sample_dist = cache.daughter_solids[i]->DistanceToIn(sample_point);
    }
    
    // Early exit if this daughter is within safety
    if (sample_dist < safety) return true;
  }
  
  // No surface found within safety distance
  return false;
}

// vim: tabstop=2 shiftwidth=2 expandtab

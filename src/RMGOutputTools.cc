// Copyright (C) 2025 Toby Dixon <toby.dixon.23@ucl.ac.uk>
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

#include "G4AffineTransform.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"

#include "RMGDetectorHit.hh"
#include "RMGLog.hh"

#include "magic_enum/magic_enum.hpp"


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
    RMGLog::Out(RMGLog::fatal, "fPositionMode is not set to kPreStep, kPostStep or kAverage instead ",
        magic_enum::enum_name<RMGOutputTools::PositionMode>(mode));

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
    RMGLog::Out(RMGLog::fatal, "fPositionMode is not set to kPreStep, kPostStep or kAverage instead ",
        magic_enum::enum_name<RMGOutputTools::PositionMode>(mode));

  return distance;
}

RMGDetectorHit* RMGOutputTools::average_hits(std::vector<RMGDetectorHit*> hits,
    bool compute_distance_to_surface, bool compute_velocity) {

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

  // time from first hit
  hit->global_time = hits.front()->global_time;

  // The cluster represents a list of consecutive steps, so we can
  // take the pre and post step from the first and last hit in the cluster.
  hit->global_position_prestep = hits.front()->global_position_prestep;
  hit->global_position_poststep = hits.back()->global_position_poststep;

  // issue: this could be outside the volume!
  hit->global_position_average = (hit->global_position_prestep + hit->global_position_poststep) / 2.;

  // compute the distance to the surface, for the pre/post step this is already done
  // but a new calculation is needed for the average.
  if (compute_distance_to_surface) {
    hit->distance_to_surface_prestep = hits.front()->distance_to_surface_prestep;
    hit->distance_to_surface_poststep = hits.back()->distance_to_surface_poststep;
    hit->distance_to_surface_average =
        distance_to_surface(hits.back()->physical_volume, hit->global_position_average);
  }
  // prestep velocity from the first hit and poststep from the last
  if (compute_velocity) {
    hit->velocity_pre = hits.front()->velocity_pre;
    hit->velocity_post = hits.back()->velocity_post;
  }

  return hit;
}

void RMGOutputTools::redistribute_gamma_energy(std::map<int, std::vector<RMGDetectorHit*>> hits_map,
    RMGOutputTools::ClusterPars cluster_pars, bool has_distance_to_surface) {

  RMGLog::Out(RMGLog::debug, "Merging gamma tracks ");

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


std::map<int, std::vector<RMGDetectorHit*>> RMGOutputTools::
    combine_low_energy_tracks(std::map<int, std::vector<RMGDetectorHit*>> hits_map,
        RMGOutputTools::ClusterPars cluster_pars, bool has_distance_to_surface) {

  RMGLog::Out(RMGLog::debug, "Merging low energy electron tracks ");


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
    double threshold =
        (not has_distance_to_surface) or
                ((input_hits.front()->distance_to_surface_prestep) > cluster_pars.surface_thickness)
            ? cluster_pars.cluster_distance
            : cluster_pars.cluster_distance_surface;

    // now search for another track to merge it with
    int cluster_trackid = -1;
    float cluster_energy = 0;

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
      output_hits[cluster_trackid].insert(output_hits[cluster_trackid].begin(),
          output_hits[trackid].begin(), output_hits[trackid].end());

      output_hits.erase(trackid);
      RMGLog::Out(RMGLog::debug, "Removing trackid ", trackid);
    }
  }
  return output_hits;
}

RMGDetectorHitsCollection* RMGOutputTools::pre_cluster_hits(const RMGDetectorHitsCollection* hits,
    RMGOutputTools::ClusterPars cluster_pars, bool has_distance_to_surface, bool has_velocity) {

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
        bool is_surface_first_hit =
            has_distance_to_surface and
            (cluster_first_hit->distance_to_surface_average < cluster_pars.surface_thickness);

        // start a new cluster if the previous step was in the surface and the new is in the bulk
        bool surface_transition = (is_surface != is_surface_first_hit);

        // get the right distance to pre-cluster
        double threshold =
            is_surface ? cluster_pars.cluster_distance_surface : cluster_pars.cluster_distance;


        // start a new cluster also if the distance is above the threshold
        start_new_cluster =
            surface_transition ||
            (hit->global_position_average - cluster_first_hit->global_position_average).mag() >=
                threshold;
      }

      // add the hit to the correct vector
      if (start_new_cluster) {
        hits_vector.push_back(std::vector<RMGDetectorHit*>());
        hits_vector.back().push_back(hit);

        cluster_first_hit = hit;
      } else {
        hits_vector.back().push_back(hit);
      }
    }
  }

  // create a container for the output hits
  auto out = new RMGDetectorHitsCollection();

  // average the hits
  for (const auto& value : hits_vector) {

    // average the hit and insert into the collection
    auto averaged_hit = average_hits(value, has_distance_to_surface, has_velocity);
    out->insert(averaged_hit);
  }

  return out;
}


double RMGOutputTools::distance_to_surface(const G4VPhysicalVolume* pv,
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

// vim: tabstop=2 shiftwidth=2 expandtab

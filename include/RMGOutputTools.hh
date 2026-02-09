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

#ifndef _RMG_OUTPUT_TOOLS_HH_
#define _RMG_OUTPUT_TOOLS_HH_

#include <map>
#include <unordered_map>
#include <vector>

#include "G4Step.hh"

#include "RMGDetectorHit.hh"
#include "RMGDetectorMetadata.hh"

/** @brief Functionality for simple output post-processing (i.e., pre-clustering and similar)
 * shared between multiple output schemes.
 */
namespace RMGOutputTools {

  /** @brief Enum of which position of the hit to store. */
  enum class PositionMode {
    /**Store the prestep point. */
    kPreStep,
    /**Store the poststep point. */
    kPostStep,
    /**Store the average. */
    kAverage,
    /**Store both post and prestep */
    kBoth,
  };


  /** @brief Container for the parameters of step pre-clustering. */
  struct ClusterPars {
      bool combine_low_energy_tracks;
      bool reassign_gamma_energy;
      double track_energy_threshold;
      double surface_thickness;
      double cluster_distance;
      double cluster_distance_surface;
      double cluster_time_threshold;
  };

  /** @brief Get the position to save for a given hit.
   *
   * @details If the mode is @c kPostStep or if the particle is a gamma
   * then the post-step point is used. This is since gamma's have only
   * discrete interactions happening at the post-step.
   * Otherwise if mode is @c kPreStep the prestep point is extracted, if it is either
   * @c kBoth or @c kAverage the average of pre and post step is used (in the case of
   * @c kBoth the pre and post step should also be saved separately).
   */
  G4ThreeVector get_position(RMGDetectorHit* hit, RMGOutputTools::PositionMode mode);

  /** @brief Get the distance to surface to save for a given hit.
   *
   * @details The logic is the same as for @ref RMGOutputTools::get_position
   */
  double get_distance(RMGDetectorHit* hit, RMGOutputTools::PositionMode mode);

  /** @brief Enable or disable Germanium-only filtering for bounding sphere optimization.
   *
   * @details When enabled, the bounding sphere optimization in distance_to_surface and
   * is_within_surface_safety will only be applied to daughter volumes that are registered
   * as Germanium detectors. This can improve performance when only Germanium detectors
   * are relevant for surface distance calculations.
   *
   * Note: Enabling this will clear the volume cache to rebuild with detector status.
   *
   * @param enable true to filter for Germanium detectors only, false to apply to all volumes
   */
  void SetDistanceCheckGermaniumOnly(bool enable);

  /** @brief Compute the distance from the point to the surface of the physical volume.
   * @details Checks distance to surfaces of mother volume.
   * @param pv The physical volume to find the distance to.
   * @param position The position to evaluate the distance for.
   */
  double distance_to_surface(const G4VPhysicalVolume* pv, const G4ThreeVector& position);

  /** @brief Check if any surface is closer than a given safety distance.
   * @details More efficient than distance_to_surface when only a threshold check is needed,
   * as it can exit early as soon as any surface is found closer than the safety.
   * @param pv The physical volume to check.
   * @param position The position to evaluate.
   * @param safety The safety distance threshold.
   * @return true if any surface (parent or daughters) is within the safety distance.
   */
  bool is_within_surface_safety(const G4VPhysicalVolume* pv, const G4ThreeVector& position, double safety);

  /** @brief Perform a basic reduction of the hits collection removing very short steps.
   *
   *  @details This is based on a "within" track clustering (but note that some low energy tracks
   * can be merged by @c CombineLowEnergyTracks .
   * The steps in every track are looped through and combined into effective steps. A step is
   * added to the current cluster if:
   *
   * - If the flag @c has_distance_to_surface is set. The step must not move from the surface region
   * (defined by the @c distance_to_surface<fSurfaceThickness) to the bulk or visa versa.
   * - the time difference between the step and the first of the cluster is not above @c
   * fClusterTimeThreshold ,
   * - the distance between the step and the first of the cluster is not above @c fClusterDistance
   * (for the bulk) or
   * @c fClusterSurfaceDistance for the surface (if @c has_distance_to_surface is true).
   * - the hits in each cluster are then combined into one effective step with @c AverageHits
   *
   * @returns a collection of hits after pre-clustering.
   */
  std::shared_ptr<RMGDetectorHitsCollection> pre_cluster_hits(
      const RMGDetectorHitsCollection* hits,
      ClusterPars cluster_pars,
      bool has_distance_to_surface,
      bool has_velocity
  );

  /** @brief Average a cluster of hits to produce one effective hit.
   *
   * @details The steps in a cluster are average with the energy being the sum over the steps,
   * and the pre/post step position / distance / velcity to surface computed from the first/last step.
   * Other fields must be the same for all steps in the cluster and are taken from the first step.
   *
   * @param hits the vector of hits to average
   * @param compute_distance_to_surface boolean flag of whether to compute the distance to surface.
   * @param compute_velocity boolean flag of whether to compute velocity.
   *
   * @returns the averaged hit.
   */
  RMGDetectorHit* average_hits(
      std::vector<RMGDetectorHit*> hits,
      bool compute_distance_to_surface,
      bool compute_velocity
  );

  /** @brief Check if the step point is contained in a physical volume registered as a detector.
   *
   * @param step_point The step point (either post or pre step) to check.
   * @param det_type Type of detector to check for.
   */
  bool check_step_point_containment(const G4StepPoint* step_point, RMGDetectorType det_type);

  /** @brief Combine low energy electron tracks into their neighbours.
   *
   *  @details Some interactions of gammas, eg. Compton scattering or the
   * photoelectric effect can produce very low energy electron tracks. This function
   * reads a map of steps in each track (keyed by trackid), it then computes the
   * total energy in each electron track.
   *  If a track is below a certain threshold then the code searches through the
   * other tracks to see if there is one where the first pre-step point is
   * within the cluster distance of this track. If so they are combined for further
   * pre-clustering. In the case multiple nearby tracks are found the highest
   * energy one is used.
   *
   * @param hits_map a map of vectors of @c RMGDetectorHit pointers with the key
   * being the track id.
   * @param cluster_pars a @ref RMGOutputTools::ClusterPars struct of the parameters for clustering.
   * @param has_distance_to_surface a flag of whether the hits have the distance to surface field,
   * and clustering should be performed separately for surface and bulk.
   *
   * @returns A map of steps after combining low energy tracks.
   */
  std::map<int, std::vector<RMGDetectorHit*>> combine_low_energy_tracks(
      const std::map<int, std::vector<RMGDetectorHit*>>& hits_map,
      const ClusterPars& cluster_pars,
      bool has_distance_to_surface
  );

  /** @brief Search for hits close to any gamma track and reassign the energy deposit to that track.
   *
   * @details Gamma particles do not deposit energy, however as part of some interactions a very
   * small "local" energy deposit happen, due to atomic binding energy. This method search through
   * the gamma tracks and for each it looks for a nearby electron hit to instead assign this small
   * local energy deposit too, this can avoid writing out the gamma tracks in the output scheme.
   */
  void redistribute_gamma_energy(
      std::map<int, std::vector<RMGDetectorHit*>> hits_map,
      ClusterPars cluster_pars,
      bool has_distance_to_surface
  );


  // Cache structure for volume geometry data
  struct VolumeCache {
      G4AffineTransform inverse_transform;
      const G4VSolid* solid;
      size_t num_daughters;
      std::vector<G4AffineTransform> daughter_transforms;
      std::vector<const G4VSolid*> daughter_solids;
      std::vector<bool> daughter_is_multiunion;
      std::vector<G4ThreeVector> daughter_centers; // bounding sphere centers in parent local coords
      std::vector<double> daughter_radii;          // bounding sphere radii
      std::vector<bool> daughter_is_germanium; // whether daughter is registered as Germanium detector
  };

  // Cache for volume data, keyed by physical volume pointer
  extern std::unordered_map<const G4VPhysicalVolume*, VolumeCache> volume_cache;

  /** @brief Add a physical volume to the cache for distance to surface calculations.
   *
   * @details This computes the inverse transform and daughter volume information for the physical
   * volume and stores it in the cache. If the volume is already in the cache, it returns an
   * iterator to the existing entry. Otherwise, it adds a new entry and returns an iterator to it.
   */
  std::unordered_map<const G4VPhysicalVolume*, VolumeCache>::iterator AddOrGetFromCache(
      const G4VPhysicalVolume* pv
  );

  // Configuration for safety distance check
  extern bool is_distance_check_germanium_only;


} // namespace RMGOutputTools

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

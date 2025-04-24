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

#ifndef _RMG_OUTPUT_TOOLS_HH_
#define _RMG_OUTPUT_TOOLS_HH_

#include <map>
#include <vector>

#include "RMGDetectorHit.hh"

namespace RMGOutputTools {

  /** @brief Container for the parameters of step pre-clustering. */
  struct ClusterPars {
      bool combine_low_energy_tracks;
      double track_energy_threshold;
      double surface_thickness;
      double cluster_distance;
      double cluster_distance_surface;
      double cluster_time_threshold;
  };

  /** @brief Compute the distance from the point to the surface of the physical volume.
   * @details Checks distance to surfaces of mother volume.
   * @param pv The physical volume to find the distance to.
   * @param position The position to evaluate the distance for.
   */
  double distance_to_surface(const G4VPhysicalVolume* pv, const G4ThreeVector& position);

  /** @brief Perform a basic reduction of the hits collection removing very short steps.
   *
   *  @details This is based on a "within" track clustering (but note that some low energy tracks
   * can be merged by @c CombineLowEnergyTracks .
   * The steps in every track are looped through and combined into effective steps. A step is
   * added to the current cluster if:
   *
   * - it does not move from the surface region (defined by the @c
   * distance_to_surface<fSurfaceThickness) to the bulk or visa versa.
   * - the time difference between the step and the first of the cluster is not above @c
   * fClusterTimeThreshold ,
   * - the distance between the step and the first of the cluster is not above @c fClusterDistance
   * (for the bulk) or
   * @c fClusterSurfaceDistance for the surface.
   * - the hits in each cluster are then combined into one effective step with @c AverageHits
   *
   * @returns a collection of hits after pre-clustering.
   */
  RMGDetectorHitsCollection* pre_cluster_hits(const RMGDetectorHitsCollection* hits,
      ClusterPars cluster_pars);

  /** @brief Average a cluster of hits to produce one effective hit.
   *
   * @details The steps in a cluster are average with the energy being the sum over the steps,
   * and the pre/post step position / distance to surface computed from the first/last step.
   * Other fields must be the same for all steps in the cluster and are taken from the first step.
   *
   * @returns the averaged hit.
   */
  RMGDetectorHit* average_hits(std::vector<RMGDetectorHit*> hits);

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
   * @returns A map of steps after combining low energy tracks.
   */
  std::map<int, std::vector<RMGDetectorHit*>> combine_low_energy_tracks(std::map<int,
                                                                            std::vector<RMGDetectorHit*>>
                                                                            hits_map,
      ClusterPars cluster_pars);

} // namespace RMGOutputTools

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

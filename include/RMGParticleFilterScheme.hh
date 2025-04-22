// Copyright (C) 2022 Eric Esch <eric.esch@uni-tuebingen.de>
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

#ifndef _RMG_PARTICLE_FILTER_SCHEME_HH_
#define _RMG_PARTICLE_FILTER_SCHEME_HH_

#include <optional>
#include <set>

#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

/** @brief Filter-output scheme for particles.
 *
 *  @details This optional output scheme filters out all particles specified via
 *  their PDG code. Optionally, it can apply this filter to a specified volume
 *  or ignore a specified volume. Properties need to be specified per macro.
 *
 *  Does nothing if the output scheme is not enabled per macro before run initialization.
 *  Also does nothing if no particle is specified. If no volume is specified the filter
 *  is applied to all volumes.
 */
class RMGParticleFilterScheme : public RMGVOutputScheme {

  public:

    RMGParticleFilterScheme();

    /** @brief Wraps @c G4UserStackingAction::StackingActionClassify
     *  @details This is used to classify all specified particles as @c fKill if
     *  they are in the specified volumes (or no volume is specified).
     *
     *  If the primary particle is filtered out here, the simulation crashes. To
     *  avoid the crash, the particle will be simulated anyways and a warning message will
     *  be shown.
     */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    /** @brief Add a particle, identified by its PDG code, to the list of particles to kill. */
    inline void AddParticle(int pdg) { fParticles.insert(pdg); }

    /** @brief Add a physical volume, by name, to the volumes in which the filter will
     *  not be applied.
     *  @details This means that specified particles outside of the specified
     *  keep-volumes will be filtered. It is therefore not possible to specify keep-volumes
     *  and kill-volumes in the same geometry.
     */
    void AddKeepVolume(std::string name);

    /** @brief Add a physical volume, by name, to the volumes in which the filter will
     *  be applied.
     *  @details This means that specified particles outside of the specified
     *  kill-volumes will not be affected by the filter. It is therefore not possible to
     *  specify keep-volumes and kill-volumes in the same geometry.
     */
    void AddKillVolume(std::string name);

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    std::set<int> fParticles;
    std::set<std::string> fKeepVolumes;
    std::set<std::string> fKillVolumes;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

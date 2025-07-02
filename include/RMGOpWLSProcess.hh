// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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


#ifndef _RMG_OP_WLS_PROCESS_
#define _RMG_OP_WLS_PROCESS_

/**
 * @brief A wrapper for the Geant4 optical wavelength shifting (WLS) process.
 *
 * This class extends @c G4WrapperProcess to customize the behavior of the standard
 * @c G4OpWLS process. It uses a material property @c RMG_WLSMEANNUMBERPHOTONS to control
 * the mean number of emitted photons and prevents the default Poissonian sampling by
 * replacing the original @c WLSMEANNUMBERPHOTONS property.
 */

#include "G4ParticleDefinition.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VParticleChange.hh"
#include "G4WrapperProcess.hh"
#include "globals.hh"

class RMGOpWLSProcess : public G4WrapperProcess {

  public:

    /**
     * @brief Constructs a new RMG Optical WLS process.
     *
     * @param aNamePrefix Prefix for naming the process (default "RMG").
     * @param aType Process type (default @c fOptical).
     */
    explicit RMGOpWLSProcess(const G4String& aNamePrefix = "RMG", G4ProcessType aType = fOptical);

    /**
     * @brief Virtual destructor.
     */
    virtual ~RMGOpWLSProcess() = default;

    /**
     * @brief Applies the custom optical WLS process in the post-step of a track.
     *
     * This method overrides @c PostStepDoIt() of @ref G4WrapperProcess. It first calls the registered
     * process's @c PostStepDoIt() to obtain the default particle change. If the returned result
     * indicates that no secondary photon has been produced and the track is stopped, the result
     * is returned unchanged. Otherwise, the method samples a random number compared to the material
     * property @c RMG_WLSMEANNUMBERPHOTONS and, if the random value exceeds this mean, it kills the
     * secondary photon by proposing a kill status. Finally, the modified particle change is returned.
     *
     * @param aTrack The current track.
     * @param aStep The current step.
     * @return Pointer to the modified particle change.
     */
    G4VParticleChange* PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) override;

    /**
     * @brief Builds the physics table for the WLS process.
     *
     * This method overrides @c BuildPhysicsTable() to customize the handling of the mean number
     * of emitted photons. After invoking the registered process's @c BuildPhysicsTable(), it
     * iterates through the global material table. For each material possessing the property
     * @c WLSMEANNUMBERPHOTONS, it replaces it with @c RMG_WLSMEANNUMBERPHOTONS to disable the
     * default Poissonian sampling. If a mean emission number greater than 1 is found, a warning is
     * issued and that material is skipped.
     *
     * @param aParticleType The particle definition for which the physics table is built.
     */
    void BuildPhysicsTable(const G4ParticleDefinition& aParticleType) override;
};
#endif

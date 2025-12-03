// Copyright (C) 2025 Zichen "Francis" Wang <https://orcid.org/0009-0007-4386-0819>
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

#ifndef _RMG_INNER_BREMSSTRAHLUNG_PROCESS_HH
#define _RMG_INNER_BREMSSTRAHLUNG_PROCESS_HH

#include "G4GenericMessenger.hh"
#include "G4ParticleDefinition.hh"
#include "G4Positron.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VParticleChange.hh"
#include "G4WrapperProcess.hh"
#include "globals.hh"

class RMGInnerBremsstrahlungProcess : public G4WrapperProcess {

  public:

    /**
     * @brief Constructs a new RMG Inner Bremsstrahlung wrapper process.
     *
     * @param aNamePrefix Prefix for naming the process (default "RMG_IB").
     * @param aType Process type (default @c fDecay).
     */
    explicit RMGInnerBremsstrahlungProcess(
        const G4String& aNamePrefix = "RMG_IB",
        G4ProcessType aType = fDecay
    );

    /**
     * @brief Virtual destructor.
     */
    virtual ~RMGInnerBremsstrahlungProcess() = default;

    /**
     * @brief Applies Inner Bremsstrahlung generation after radioactive decay.
     *
     * This method overrides @c PostStepDoIt() of @ref G4WrapperProcess. It first calls the
     * wrapped decay process's @c PostStepDoIt() to obtain the decay products. Then it examines
     * all secondary particles, identifies beta electrons, calculates the Inner Bremsstrahlung
     * probability based on the electron energy, and generates IB photons accordingly. The IB
     * photons are created with proper kinematic properties and added to the secondary stack.
     *
     * @param aTrack The current track undergoing decay.
     * @param aStep The current step.
     * @return Pointer to the particle change with added IB photons.
     */
    G4VParticleChange* AtRestDoIt(const G4Track& aTrack, const G4Step& aStep) override;

    /**
     * @brief Applies Inner Bremsstrahlung generation after radioactive decay during step.
     *
     * This method overrides @c PostStepDoIt() of @ref G4WrapperProcess. It first calls the
     * wrapped decay process's @c PostStepDoIt() to obtain the decay products. Then it examines
     * all secondary particles, identifies beta electrons, calculates the Inner Bremsstrahlung
     * probability based on the electron energy, and generates IB photons accordingly. The IB
     * photons are created with proper kinematic properties and added to the secondary stack.
     *
     * @param aTrack The current track undergoing decay.
     * @param aStep The current step.
     * @return Pointer to the particle change with added IB photons.
     */

    G4VParticleChange* PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) override;

    /**
     * @brief Enables or disables Inner Bremsstrahlung generation.
     *
     * @param enabled True to enable IB generation, false to disable.
     */
    void SetEnabled(G4bool enabled) { fEnabled = enabled; }

    /**
     * @brief Checks if Inner Bremsstrahlung generation is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] G4bool IsEnabled() const { return fEnabled; }

    /**
     * @brief Sets a scaling factor for IB probability (for systematic studies).
     *
     * @param scale Probability scaling factor (default 1.0).
     */
    void SetIBProbabilityScale(G4double scale) { fIBProbabilityScale = scale; }

    /**
     * @brief Gets the current IB probability scaling factor.
     *
     * @return Probability scaling factor.
     */
    [[nodiscard]] G4double GetIBProbabilityScale() const { return fIBProbabilityScale; }

  private:

    /**
     * @brief Calculates the phi function for Inner Bremsstrahlung spectrum.
     *
     * @param W_prime Dimensionless electron energy before photon emission.
     * @param omega Dimensionless photon energy.
     * @return Value of the phi function.
     */
    G4double PhiFunction(G4double W_prime, G4double omega);

    /**
     * @brief Calculates the total Inner Bremsstrahlung probability for a given electron energy.
     *
     * @param electronEnergy Kinetic energy of the beta electron.
     * @return Total IB probability (0 to 1).
     */
    G4double CalculateIBProbability(G4double electronEnergy);

    /**
     * @brief Samples a photon energy from the Inner Bremsstrahlung spectrum.
     *
     * @param electronEnergy Kinetic energy of the beta electron.
     * @return Sampled IB photon energy.
     */
    G4double SamplePhotonEnergy(G4double electronEnergy);

    /**
     * @brief Generates IB photons for beta electrons in the decay secondaries.
     *
     * @param particleChange Particle change object containing decay secondaries.
     * @param parentTrack The original decaying nucleus track.
     * @param aStep The current step.
     */
    void GenerateInnerBremsstrahlungForSecondaries(
        G4VParticleChange* particleChange,
        const G4Track& parentTrack,
        const G4Step& aStep
    );

    /**
     * @brief Checks if a secondary track is a beta electron from decay.
     *
     * @param track Secondary track to check.
     * @return True if it's a beta electron, false otherwise.
     */
    G4bool IsBetaElectron(G4Track* track);

    // Physical constants
    G4bool fEnabled;
    G4double fIBProbabilityScale;

    // messenger stuff
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

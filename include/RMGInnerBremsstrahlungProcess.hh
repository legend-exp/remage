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

#include <memory>
#include <vector>

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
    void SetEnabled(bool enabled) { fEnabled = enabled; }

    /**
     * @brief Checks if Inner Bremsstrahlung generation is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] bool IsEnabled() const { return fEnabled; }

    /**
     * @brief Sets a scaling factor for IB probability (for systematic studies).
     * @details The factor is shared across all per-isotope process instances.
     *
     * @param factor Probability scaling factor (default 1.0).
     */
    void SetBiasingFactor(double factor) { fBiasingFactor = factor; }

    /**
     * @brief Gets the current IB probability scaling factor.
     *
     * @return Probability scaling factor.
     */
    [[nodiscard]] double GetBiasingFactor() const { return fBiasingFactor; }

  private:

    /**
     * @brief Calculates the phi function for Inner Bremsstrahlung spectrum.
     *
     * @param W_prime Dimensionless electron energy before photon emission.
     * @param omega Dimensionless photon energy.
     * @return Value of the phi function.
     */
    double PhiFunction(double W_prime, double omega);

    /**
     * @brief Computes the Inner Bremsstrahlung spectrum for a given electron energy.
     * @details Fills @p omegas (dimensionless photon-energy grid) and @p cdf (cumulative
     * trapezoidal integral of the spectrum up to each grid point). The same arrays are reused for
     * photon-energy sampling, so the spectrum is integrated only once per emission.
     *
     * @param electron_energy Kinetic energy of the beta electron.
     * @param omegas Output: dimensionless photon-energy grid.
     * @param cdf Output: cumulative (unnormalized) distribution over @p omegas.
     * @return Total IB probability (integral of the spectrum).
     */
    double ComputeIBSpectrum(
        double electron_energy,
        std::vector<double>& omegas,
        std::vector<double>& cdf
    );

    /**
     * @brief Samples a photon energy from a precomputed Inner Bremsstrahlung spectrum.
     *
     * @param omegas Dimensionless photon-energy grid, as filled by @c ComputeIBSpectrum.
     * @param cdf Cumulative distribution over @p omegas.
     * @return Sampled IB photon energy.
     */
    double SamplePhotonEnergy(const std::vector<double>& omegas, const std::vector<double>& cdf);

    /**
     * @brief Generates IB photons for beta electrons in the decay secondaries.
     *
     * @param particle_change Particle change object containing decay secondaries.
     * @param parent_track The original decaying nucleus track.
     */
    void GenerateInnerBremsstrahlungForSecondaries(
        G4VParticleChange* particle_change,
        const G4Track& parent_track
    );

    /**
     * @brief Checks if a secondary track is a beta electron from decay.
     *
     * @param track Secondary track to check.
     * @return True if it's a beta electron, false otherwise.
     */
    bool IsBetaElectron(G4Track* track);

    // messenger stuff. Static so that only one messenger is registered at the shared command path,
    // regardless of how many per-isotope process instances are constructed.
    inline static std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    inline static bool fEnabled = true;
    inline static double fBiasingFactor = 1.0;
};

#endif

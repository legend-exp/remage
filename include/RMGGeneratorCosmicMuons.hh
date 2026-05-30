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

#ifndef _RMG_GENERATOR_COSMIC_MUONS_HH_
#define _RMG_GENERATOR_COSMIC_MUONS_HH_

#include <memory>
#include <string>

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"

#include "RMGVGenerator.hh"

class EcoMug;

namespace u = CLHEP;
/**
 * @brief Cosmic-muon primary generator backed by the EcoMug sampler.
 *
 * Generates atmospheric muons sampled either from a horizontal plane or from the upper
 * hemisphere of a sphere (see @ref SkyShape). Momentum, zenith/azimuth and source-position
 * ranges are configurable through messenger commands. Vertex position is sampled internally
 * by EcoMug, so @ref SetParticlePosition is a no-op.
 *
 * Based on EcoMug, Pagano et al., NIM A 1014 (2021) 165732, https://doi.org/10.1016/j.nima.2021.165732.
 */
class RMGGeneratorCosmicMuons : public RMGVGenerator {

  public:

    /** @brief Geometric shape of the surface from which muons are sampled. */
    enum class SkyShape {
      kPlane, ///< Horizontal square plane at a fixed height.
      kSphere ///< Upper hemisphere of a sphere centred at the origin.
    };

    RMGGeneratorCosmicMuons();
    ~RMGGeneratorCosmicMuons();

    RMGGeneratorCosmicMuons(RMGGeneratorCosmicMuons const&) = delete;
    RMGGeneratorCosmicMuons& operator=(RMGGeneratorCosmicMuons const&) = delete;
    RMGGeneratorCosmicMuons(RMGGeneratorCosmicMuons&&) = delete;
    RMGGeneratorCosmicMuons& operator=(RMGGeneratorCosmicMuons&&) = delete;

    /** @brief Sample a muon from EcoMug and fire it through the internal particle gun. */
    void GeneratePrimaries(G4Event*) override;
    /** @brief No-op: vertex sampling is owned by EcoMug. */
    void SetParticlePosition(G4ThreeVector) override {}

    /** @brief Configure EcoMug with the user-supplied sky shape and kinematic ranges. */
    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override {}

  private:

    std::unique_ptr<EcoMug> fEcoMug;
    std::unique_ptr<G4ParticleGun> fGun = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();
    void SetSkyShape(std::string shape);

    SkyShape fSkyShape = SkyShape::kSphere;
    double fSkyPlaneSize = -1;
    double fSkyPlaneHeight = 50 * u::m;
    double fSkyHSphereRadius = 50 * u::m;

    double fSpherePositionThetaMin = 0 * u::deg;
    double fSpherePositionThetaMax = 90 * u::deg;
    double fSpherePositionPhiMin = 0 * u::deg;
    double fSpherePositionPhiMax = 360 * u::deg;

    double fMomentumMin = 0 * u::GeV;
    double fMomentumMax = 1 * u::TeV;
    double fThetaMin = 0 * u::deg;
    double fThetaMax = 90 * u::deg;
    double fPhiMin = 0 * u::deg;
    double fPhiMax = 360 * u::deg;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

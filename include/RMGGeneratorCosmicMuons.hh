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
/* @brief Generate cosmic muons, based on https://doi.org/10.1016/j.nima.2021.165732 */
class RMGGeneratorCosmicMuons : public RMGVGenerator {

  public:

    enum class SkyShape {
      kPlane,
      kSphere
    };

    RMGGeneratorCosmicMuons();
    ~RMGGeneratorCosmicMuons();

    RMGGeneratorCosmicMuons(RMGGeneratorCosmicMuons const&) = delete;
    RMGGeneratorCosmicMuons& operator=(RMGGeneratorCosmicMuons const&) = delete;
    RMGGeneratorCosmicMuons(RMGGeneratorCosmicMuons&&) = delete;
    RMGGeneratorCosmicMuons& operator=(RMGGeneratorCosmicMuons&&) = delete;

    void GeneratePrimaries(G4Event*) override;
    void SetParticlePosition(G4ThreeVector) override {}

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
    double fSkyPlaneHeight = 50;
    double fSkyHSphereRadius = 50;

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

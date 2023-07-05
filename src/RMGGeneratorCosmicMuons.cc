// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
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

#include "RMGGeneratorCosmicMuons.hh"

#include <cmath>

#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleMomentum.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGTools.hh"

#include "EcoMug/EcoMug.h"
#include "magic_enum/magic_enum.hpp"

namespace u = CLHEP;

RMGGeneratorCosmicMuons::RMGGeneratorCosmicMuons() : RMGVGenerator("CosmicMuons") {
  this->DefineCommands();

  // docs: https://doi.org/10.1016/j.nima.2021.165732
  fEcoMug = std::make_unique<EcoMug>();
  fGun = std::make_unique<G4ParticleGun>();
}

// Need non-inline, i.e. not in header/class body, destructor to hide EcoMug from consumers
RMGGeneratorCosmicMuons::~RMGGeneratorCosmicMuons() = default; // NOLINT

void RMGGeneratorCosmicMuons::BeginOfRunAction(const G4Run*) {

  // TODO: get this info from detector construction
  const auto world_side = fSkyPlaneHeight; /*50* u::m*/

  RMGLog::Out(RMGLog::debug, "Configuring EcoMug");

  // TODO: add sphere?
  switch (fSkyShape) {
    case SkyShape::kPlane: {
      fEcoMug->SetUseSky();
      // put sky exactly on the top of the world
      fEcoMug->SetSkyCenterPosition({0, 0, world_side / u::m});
      auto sky_size = fSkyPlaneSize > 0 ? fSkyPlaneSize : world_side;
      fEcoMug->SetSkySize({sky_size / u::m, sky_size / u::m});
      break;
    }
    default: {
      RMGLog::OutFormat(RMGLog::fatal, "\"{}\" sky shape not implemented!",
          magic_enum::enum_name<SkyShape>(fSkyShape));
      break;
    }
  }

  fEcoMug->SetMinimumMomentum(fMomentumMin / u::GeV);
  fEcoMug->SetMaximumMomentum(fMomentumMax / u::GeV);
  fEcoMug->SetMinimumTheta(fThetaMin / u::rad);
  fEcoMug->SetMaximumTheta(fThetaMax / u::rad);
  fEcoMug->SetMinimumPhi(fPhiMin / u::rad);
  fEcoMug->SetMaximumPhi(fPhiMax / u::rad);
  fEcoMug->SetHSphereMinPositionTheta(fSpherePositionThetaMin / u::rad);
  fEcoMug->SetHSphereMaxPositionTheta(fSpherePositionThetaMax / u::rad);
  fEcoMug->SetHSphereMinPositionPhi(fSpherePositionPhiMin / u::rad);
  fEcoMug->SetHSphereMaxPositionPhi(fSpherePositionPhiMax / u::rad);

  // FIXME: somehow this always sets the same seed
  // RMGLog::OutFormat(RMGLog::debug, "EcoMug random seed: {}", CLHEP::HepRandom::getTheSeed());
  // fEcoMug->SetSeed(CLHEP::HepRandom::getTheSeed());
}

void RMGGeneratorCosmicMuons::GeneratePrimaries(G4Event* event) {

  fEcoMug->Generate();

  RMGLog::OutFormat(RMGLog::debug, "Generated µ has charge {:+}", fEcoMug->GetCharge());
  fGun->SetNumberOfParticles(1);
  fEcoMug->GetCharge() < 0 ? fGun->SetParticleDefinition(G4MuonMinus::Definition())
                           : fGun->SetParticleDefinition(G4MuonPlus::Definition());

  const auto& pos = fEcoMug->GetGenerationPosition(); // no units, the user can decide. I use meters
  RMGLog::OutFormat(RMGLog::debug, "...origin ({:.4g}, {:.4g}, {:.4g}) m", pos[0], pos[1], pos[2]);
  fGun->SetParticlePosition({pos[0] * u::m, pos[1] * u::m, pos[2] * u::m});

  G4ThreeVector d_cart(1, 1, 1);
  d_cart.setTheta(fEcoMug->GetGenerationTheta()); // in rad
  d_cart.setPhi(fEcoMug->GetGenerationPhi());     // in rad
  d_cart.setMag(1 * u::m);
  fGun->SetParticleMomentumDirection(d_cart);

  RMGLog::OutFormat(RMGLog::debug, "...direction (θ,φ) = ({:.4g}, {:.4g}) deg",
      fEcoMug->GetGenerationTheta() / u::deg, fEcoMug->GetGenerationPhi() / u::deg);
  RMGLog::OutFormat(RMGLog::debug, "...direction (x,y,z) = ({:.4g}, {:.4g}, {:.4g}) m",
      d_cart.getX() / u::m, d_cart.getY() / u::m, d_cart.getZ() / u::m);

  const auto& p_tot = fEcoMug->GetGenerationMomentum() * u::GeV;
  RMGLog::OutFormat(RMGLog::debug, "...momentum {:.4g} GeV/c", p_tot / u::GeV);
  const auto& mu_mass = G4MuonPlus::Definition()->GetPDGMass();
  fGun->SetParticleEnergy(std::sqrt(p_tot * p_tot + mu_mass * mu_mass) - mu_mass);

  fGun->GeneratePrimaryVertex(event);
}

void RMGGeneratorCosmicMuons::SetSkyShape(std::string shape) {
  try {
    fSkyShape = RMGTools::ToEnum<RMGGeneratorCosmicMuons::SkyShape>(shape, "sky shape");
  } catch (const std::bad_cast&) { return; }
}

void RMGGeneratorCosmicMuons::DefineCommands() {

  // NOTE: SetUnit(Category) is not thread-safe

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/CosmicMuons/",
      "Commands for controlling the µ generator");

  fMessenger->DeclareMethod("SkyShape", &RMGGeneratorCosmicMuons::SetSkyShape)
      .SetGuidance("Geometrical shape of the µ generation surface")
      .SetParameterName("shape", false)
      .SetCandidates(RMGTools::GetCandidates<RMGGeneratorCosmicMuons::SkyShape>())
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("SkyPlaneSize", "m", fSkyPlaneSize)
      .SetGuidance("Length of the side of the sky, if it has a planar shape")
      .SetParameterName("l", false)
      .SetRange("l > 0")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("SkyPlaneHeight", "m", fSkyPlaneHeight)
      .SetGuidance("Height of the sky, if it has a planar shape")
      .SetParameterName("l", false)
      .SetRange("l > 0")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("MomentumMin", "GeV/c", fMomentumMin)
      .SetGuidance("Minimum momentum of the generated muon")
      .SetParameterName("p", false)
      .SetRange("p >= 0 && p < 1000")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("MomentumMax", "GeV/c", fMomentumMax)
      .SetGuidance("Maximum momentum of the generated muon")
      .SetParameterName("p", false)
      .SetRange("p > 0 && p <= 1000")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("ThetaMin", "deg", fThetaMin)
      .SetGuidance("Minimum azimutal angle of the generated muon momentum")
      .SetParameterName("a", false)
      .SetRange("a >= 0 && a < 90")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("ThetaMax", "deg", fThetaMax)
      .SetGuidance("Maximum azimutal angle of the generated muon momentum")
      .SetParameterName("a", false)
      .SetRange("a > 0 && a <= 90")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("PhiMin", "deg", fPhiMin)
      .SetGuidance("Minimum zenith angle of the generated muon momentum")
      .SetParameterName("a", false)
      .SetRange("a >= 0 && a < 360")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("PhiMax", "deg", fPhiMax)
      .SetGuidance("Maximum zenith angle of the generated muon momentum")
      .SetParameterName("a", false)
      .SetRange("a > 0 && a <= 360")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("SpherePositionThetaMin", "deg", fSpherePositionThetaMin)
      .SetGuidance("Minimum azimutal angle of the generated muon position on the sphere")
      .SetParameterName("a", false)
      .SetRange("a >= 0 && a < 90")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("SpherePositionThetaMax", "deg", fSpherePositionThetaMax)
      .SetGuidance("Maximum azimutal angle of the generated muon position on the sphere")
      .SetParameterName("a", false)
      .SetRange("a > 0 && a <= 90")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("SpherePositionPhiMin", "deg", fSpherePositionPhiMin)
      .SetGuidance("Minimum zenith angle of the generated muon position on the sphere")
      .SetParameterName("a", false)
      .SetRange("a >= 0 && a < 360")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessenger->DeclarePropertyWithUnit("SpherePositionPhiMax", "deg", fSpherePositionPhiMax)
      .SetGuidance("Maximum zenith angle of the generated muon position on the sphere")
      .SetParameterName("a", false)
      .SetRange("a > 0 && a <= 360")
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

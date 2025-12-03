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

#include "RMGInnerBremsstrahlungProcess.hh"

#include <algorithm>
#include <cmath>
#include <vector>

#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4EventManager.hh"
#include "G4Gamma.hh"
#include "G4PhysicalConstants.hh"
#include "G4Positron.hh"
#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4VParticleChange.hh"
#include "Randomize.hh"

#include "RMGLog.hh"

RMGInnerBremsstrahlungProcess::RMGInnerBremsstrahlungProcess(
    const G4String& aNamePrefix,
    G4ProcessType aType
)
    : G4WrapperProcess(aNamePrefix, aType) {
  fEnabled = true;
  fIBProbabilityScale = 1.0;
  RMGLog::OutFormat(
      RMGLog::detail,
      "{}: Inner Bremsstrahlung wrapper process initialized",
      GetProcessName()
  );

  this->DefineCommands();
}

G4VParticleChange* RMGInnerBremsstrahlungProcess::AtRestDoIt(const G4Track& aTrack, const G4Step& aStep) {
  auto particleChange = pRegProcess->AtRestDoIt(aTrack, aStep);

  // If IB is disabled or no secondaries produced, return unchanged
  if (!fEnabled || particleChange->GetNumberOfSecondaries() == 0) { return particleChange; }

  RMGLog::OutFormat(RMGLog::debug, "{}: Processing decay at rest", GetProcessName());

  // Generate Inner Bremsstrahlung for any beta electrons in the secondaries
  GenerateInnerBremsstrahlungForSecondaries(particleChange, aTrack, aStep);

  return particleChange;
}


G4VParticleChange* RMGInnerBremsstrahlungProcess::PostStepDoIt(
    const G4Track& aTrack,
    const G4Step& aStep
) {
  auto particleChange = pRegProcess->PostStepDoIt(aTrack, aStep);


  // If IB is disabled or no secondaries produced, return unchanged
  if (!fEnabled || particleChange->GetNumberOfSecondaries() == 0) { return particleChange; }

  RMGLog::OutFormat(RMGLog::debug, "{}: Processing decay", GetProcessName());

  // Generate Inner Bremsstrahlung for any beta electrons in the secondaries
  GenerateInnerBremsstrahlungForSecondaries(particleChange, aTrack, aStep);

  return particleChange;
}

void RMGInnerBremsstrahlungProcess::GenerateInnerBremsstrahlungForSecondaries(
    G4VParticleChange* particleChange,
    const G4Track& parentTrack,
    const G4Step&
) {

  // First check if this decay actually produces electrons/positrons (beta decay)
  bool hasBetaParticles = false;
  for (G4int i = 0; i < particleChange->GetNumberOfSecondaries(); i++) {
    G4Track* track = particleChange->GetSecondary(i);
    if (IsBetaElectron(track)) {
      hasBetaParticles = true;
      break;
    }
  }

  if (!hasBetaParticles) {
    RMGLog::OutFormat(
        RMGLog::debug,
        "{}: No beta particles found, skipping IB generation",
        GetProcessName()
    );
    return;
  }

  // Loop through all secondaries to find beta electrons
  for (G4int i = 0; i < particleChange->GetNumberOfSecondaries(); i++) {
    G4Track* secondaryTrack = particleChange->GetSecondary(i);

    if (IsBetaElectron(secondaryTrack)) {
      G4double electronEnergy = secondaryTrack->GetKineticEnergy();
      if (electronEnergy < 0.0 * CLHEP::keV) { continue; }

      // Calculate IB probability (with optional scaling)
      G4double ibProbability = CalculateIBProbability(electronEnergy) * fIBProbabilityScale;

      RMGLog::OutFormat(
          RMGLog::debug,
          "{}: Beta electron energy: {:.3f} keV, IB probability: {:.6f}",
          GetProcessName(),
          electronEnergy / CLHEP::keV,
          ibProbability
      );

      // Sample whether IB occurs
      if (G4UniformRand() < ibProbability) {
        // Sample photon energy from IB spectrum
        G4double gammaEnergy = SamplePhotonEnergy(electronEnergy);

        // Generate random direction for the gamma (isotropic)
        G4double cosTheta = 2.0 * G4UniformRand() - 1.0;
        G4double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
        G4double phi = twopi * G4UniformRand();

        G4ThreeVector direction;
        direction.setX(sinTheta * std::cos(phi));
        direction.setY(sinTheta * std::sin(phi));
        direction.setZ(cosTheta);

        // Get position and time from the decay location
        G4ThreeVector position = secondaryTrack->GetPosition();
        G4double time = secondaryTrack->GetGlobalTime();
        G4TouchableHandle touchable = secondaryTrack->GetTouchableHandle();

        // Create the IB gamma ray as an additional secondary
        auto dynParticle = new G4DynamicParticle(G4Gamma::Definition(), direction, gammaEnergy);
        auto ibGammaTrack = new G4Track(dynParticle, time, position);
        ibGammaTrack->SetTouchableHandle(touchable);
        ibGammaTrack->SetParentID(parentTrack.GetTrackID()); // Same parent as decay
        ibGammaTrack->SetTrackStatus(fAlive);

        // Add the IB gamma to the stepping manager's secondary stack
        G4SteppingManager* steppingManager = G4EventManager::GetEventManager()
                                                 ->GetTrackingManager()
                                                 ->GetSteppingManager();
        steppingManager->GetfSecondary()->push_back(ibGammaTrack);

        RMGLog::OutFormat(
            RMGLog::debug,
            "{}: Generated IB photon {:.3f} keV from beta {:.3f} keV",
            GetProcessName(),
            gammaEnergy / CLHEP::keV,
            electronEnergy / CLHEP::keV
        );
      }
    }
  }
}

G4bool RMGInnerBremsstrahlungProcess::IsBetaElectron(G4Track* track) {
  // Check if this secondary is an electron or positron (beta particles)
  return (
      track->GetDefinition() == G4Electron::Definition() ||
      track->GetDefinition() == G4Positron::Definition()
  );
}

G4double RMGInnerBremsstrahlungProcess::PhiFunction(G4double W_prime, G4double omega) {
  // Ensure W is physically meaningful
  G4double W = W_prime - omega;
  if (W <= 1.0) // Below rest mass energy
    return 0.0;

  G4double p = std::sqrt(W * W - 1.0); // Momentum of electron after photon emission
  G4double p_prime = std::sqrt(W_prime * W_prime - 1.0); // Momentum before photon emission

  if (p_prime <= 0.0 || omega <= 0.0) return 0.0;

  G4double bracket_term = ((W * W + W_prime * W_prime) / (W_prime * p)) * std::log(W + p) - 2.0;

  G4double result = (CLHEP::fine_structure_const * p) / (pi * omega * p_prime) * bracket_term;
  return std::max(0.0, result); // Ensure non-negative results
}

G4double RMGInnerBremsstrahlungProcess::CalculateIBProbability(G4double electronEnergy) {
  // Convert electron energy to dimensionless units
  G4double W_prime = electronEnergy / CLHEP::electron_mass_c2 + 1.0;

  if (W_prime <= 1.0) return 0.0;

  // Integration parameters
  const int numPoints = 100;
  G4double maxOmega = W_prime - 1.0 - 0.01; // Leave some margin

  if (maxOmega <= 0.01) return 0.0;

  G4double deltaOmega = maxOmega / numPoints;

  // Simple numerical integration (trapezoidal rule)
  G4double totalProb = 0.0;
  for (int i = 0; i < numPoints; i++) {
    G4double omega1 = 0.01 + i * deltaOmega;
    G4double omega2 = 0.01 + (i + 1) * deltaOmega;

    if (omega1 < 0.01) omega1 = 0.01;

    G4double phi1 = PhiFunction(W_prime, omega1);
    G4double phi2 = PhiFunction(W_prime, omega2);

    totalProb += 0.5 * (phi1 + phi2) * deltaOmega;
  }

  return totalProb;
}

G4double RMGInnerBremsstrahlungProcess::SamplePhotonEnergy(G4double electronEnergy) {
  // Convert electron energy to dimensionless units
  G4double W_prime = electronEnergy / CLHEP::electron_mass_c2 + 1.0;

  // Create a cumulative distribution function (CDF) for sampling
  const int numPoints = 100;
  G4double maxOmega = W_prime - 1.0 - 0.01; // Leave some margin

  if (maxOmega <= 0.01) return 0.01 * CLHEP::electron_mass_c2;

  G4double deltaOmega = maxOmega / numPoints;

  std::vector<G4double> omegas;
  std::vector<G4double> cdf;

  G4double sum = 0.0;

  // Calculate the spectrum values and construct the CDF
  for (int i = 0; i < numPoints; i++) {
    G4double omega = 0.01 + i * deltaOmega;
    G4double value = PhiFunction(W_prime, omega);

    omegas.push_back(omega);
    sum += value;
    cdf.push_back(sum);
  }

  // Normalize the CDF
  if (sum > 0.0) {
    for (double& i : cdf) { i /= sum; }
  } else {
    return 0.01 * CLHEP::electron_mass_c2; // Fallback
  }

  // Sample from the CDF
  G4double r = G4UniformRand();
  for (size_t i = 0; i < cdf.size(); i++) {
    if (r <= cdf[i]) {
      // Convert back to energy in keV
      return omegas[i] * CLHEP::electron_mass_c2;
    }
  }

  // Fallback to the last value
  return omegas.back() * CLHEP::electron_mass_c2;
}

void RMGInnerBremsstrahlungProcess::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Processes/InnerBremsstrahlung/",
      "Commands for controlling the inner bremsstrahlung process"
  );

  fMessenger->DeclareMethod("BiasingFactor", &RMGInnerBremsstrahlungProcess::SetIBProbabilityScale)
      .SetGuidance("Sets a scaling factor for IB probability")
      .SetParameterName("factor", false)
      .SetStates(G4State_PreInit, G4State_Idle);
}

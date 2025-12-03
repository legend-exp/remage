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
  RMGLog::OutFormat(
      RMGLog::detail,
      "{}: Inner Bremsstrahlung wrapper process initialized",
      GetProcessName()
  );

  this->DefineCommands();
}

G4VParticleChange* RMGInnerBremsstrahlungProcess::AtRestDoIt(const G4Track& aTrack, const G4Step& aStep) {
  auto particle_change = pRegProcess->AtRestDoIt(aTrack, aStep);

  // If IB is disabled or no secondaries produced, return unchanged
  if (!fEnabled || particle_change->GetNumberOfSecondaries() == 0) { return particle_change; }

  RMGLog::OutFormat(RMGLog::debug, "{}: Processing decay at rest", GetProcessName());

  // Generate Inner Bremsstrahlung for any beta electrons in the secondaries
  GenerateInnerBremsstrahlungForSecondaries(particle_change, aTrack);

  return particle_change;
}


G4VParticleChange* RMGInnerBremsstrahlungProcess::PostStepDoIt(
    const G4Track& aTrack,
    const G4Step& aStep
) {
  auto particle_change = pRegProcess->PostStepDoIt(aTrack, aStep);

  // If IB is disabled or no secondaries produced, return unchanged
  if (!fEnabled || particle_change->GetNumberOfSecondaries() == 0) { return particle_change; }

  RMGLog::OutFormat(RMGLog::debug, "{}: Processing decay", GetProcessName());

  // Generate Inner Bremsstrahlung for any beta electrons in the secondaries
  GenerateInnerBremsstrahlungForSecondaries(particle_change, aTrack);

  return particle_change;
}

void RMGInnerBremsstrahlungProcess::GenerateInnerBremsstrahlungForSecondaries(
    G4VParticleChange* particle_change,
    const G4Track& parent_track
) {

  // First check if this decay actually produces electrons/positrons (beta decay)
  bool has_beta_particles = false;
  for (int i = 0; i < particle_change->GetNumberOfSecondaries(); i++) {
    auto track = particle_change->GetSecondary(i);
    if (IsBetaElectron(track)) {
      has_beta_particles = true;
      break;
    }
  }

  if (!has_beta_particles) {
    RMGLog::OutFormat(
        RMGLog::debug,
        "{}: No beta particles found, skipping IB generation",
        GetProcessName()
    );
    return;
  }

  // Loop through all secondaries to find beta electrons
  for (int i = 0; i < particle_change->GetNumberOfSecondaries(); i++) {
    auto secondary_track = particle_change->GetSecondary(i);

    if (IsBetaElectron(secondary_track)) {
      auto electron_energy = secondary_track->GetKineticEnergy();
      if (electron_energy < 0.0 * CLHEP::keV) { continue; }

      // Calculate IB probability (with optional scaling)
      auto ib_probability = CalculateIBProbability(electron_energy) * fBiasingFactor;

      RMGLog::OutFormat(
          RMGLog::debug,
          "{}: Beta electron energy: {:.3f} keV, IB probability: {:.6f}",
          GetProcessName(),
          electron_energy / CLHEP::keV,
          ib_probability
      );

      // Sample whether IB occurs
      if (G4UniformRand() < ib_probability) {
        // Sample photon energy from IB spectrum
        auto gamma_energy = SamplePhotonEnergy(electron_energy);

        // Generate random direction for the gamma (isotropic)
        double cos_theta = 2.0 * G4UniformRand() - 1.0;
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
        double phi = twopi * G4UniformRand();

        G4ThreeVector direction;
        direction.setX(sin_theta * std::cos(phi));
        direction.setY(sin_theta * std::sin(phi));
        direction.setZ(cos_theta);

        // Get position and time from the decay location
        auto position = secondary_track->GetPosition();
        auto time = secondary_track->GetGlobalTime();
        auto touchable = secondary_track->GetTouchableHandle();

        // Create the IB gamma ray as an additional secondary
        auto dyn_particle = new G4DynamicParticle(G4Gamma::Definition(), direction, gamma_energy);
        auto ib_gamma_track = new G4Track(dyn_particle, time, position);
        ib_gamma_track->SetTouchableHandle(touchable);
        ib_gamma_track->SetParentID(parent_track.GetTrackID()); // Same parent as decay
        ib_gamma_track->SetTrackStatus(fAlive);

        // Add the IB gamma to the stepping manager's secondary stack
        auto stepping_manager = G4EventManager::GetEventManager()
                                    ->GetTrackingManager()
                                    ->GetSteppingManager();
        stepping_manager->GetfSecondary()->push_back(ib_gamma_track);

        RMGLog::OutFormat(
            RMGLog::debug,
            "{}: Generated IB photon {:.3f} keV from beta {:.3f} keV",
            GetProcessName(),
            gamma_energy / CLHEP::keV,
            electron_energy / CLHEP::keV
        );
      }
    }
  }
}

bool RMGInnerBremsstrahlungProcess::IsBetaElectron(G4Track* track) {
  // Check if this secondary is an electron or positron (beta particles)
  return (
      track->GetDefinition() == G4Electron::Definition() ||
      track->GetDefinition() == G4Positron::Definition()
  );
}

double RMGInnerBremsstrahlungProcess::PhiFunction(double W_prime, double omega) {
  // Ensure W is physically meaningful
  double W = W_prime - omega;
  if (W <= 1.0) // Below rest mass energy
    return 0.0;

  double p = std::sqrt(W * W - 1.0);                   // Momentum of electron after photon emission
  double p_prime = std::sqrt(W_prime * W_prime - 1.0); // Momentum before photon emission

  if (p_prime <= 0.0 || omega <= 0.0) return 0.0;

  double bracket_term = ((W * W + W_prime * W_prime) / (W_prime * p)) * std::log(W + p) - 2.0;

  double result = (CLHEP::fine_structure_const * p) / (pi * omega * p_prime) * bracket_term;
  return std::max(0.0, result); // Ensure non-negative results
}

double RMGInnerBremsstrahlungProcess::CalculateIBProbability(double electron_energy) {
  // Convert electron energy to dimensionless units
  double W_prime = electron_energy / CLHEP::electron_mass_c2 + 1.0;

  if (W_prime <= 1.0) return 0.0;

  // Integration parameters
  const int num_points = 100;
  double max_omega = W_prime - 1.0 - 0.01; // Leave some margin

  if (max_omega <= 0.01) return 0.0;

  double delta_omega = max_omega / num_points;

  // Simple numerical integration (trapezoidal rule)
  double totalProb = 0.0;
  for (int i = 0; i < num_points; i++) {
    double omega1 = 0.01 + i * delta_omega;
    double omega2 = 0.01 + (i + 1) * delta_omega;

    if (omega1 < 0.01) omega1 = 0.01;

    double phi1 = PhiFunction(W_prime, omega1);
    double phi2 = PhiFunction(W_prime, omega2);

    totalProb += 0.5 * (phi1 + phi2) * delta_omega;
  }

  return totalProb;
}

double RMGInnerBremsstrahlungProcess::SamplePhotonEnergy(double electron_energy) {
  // Convert electron energy to dimensionless units
  double W_prime = electron_energy / CLHEP::electron_mass_c2 + 1.0;

  // Create a cumulative distribution function (CDF) for sampling
  const int num_points = 100;
  double max_omega = W_prime - 1.0 - 0.01; // Leave some margin

  if (max_omega <= 0.01) return 0.01 * CLHEP::electron_mass_c2;

  double delta_omega = max_omega / num_points;

  std::vector<double> omegas;
  std::vector<double> cdf;

  double sum = 0.0;

  // Calculate the spectrum values and construct the CDF
  for (int i = 0; i < num_points; i++) {
    double omega = 0.01 + i * delta_omega;
    double value = PhiFunction(W_prime, omega);

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
  double r = G4UniformRand();
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

  fMessenger->DeclareMethod("BiasingFactor", &RMGInnerBremsstrahlungProcess::SetBiasingFactor)
      .SetGuidance("Sets a biasing factor for IB probability")
      .SetParameterName("factor", false)
      .SetStates(G4State_PreInit, G4State_Idle);
}

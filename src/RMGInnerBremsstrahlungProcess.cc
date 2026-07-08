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
#include <mutex>
#include <vector>

#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4PhysicalConstants.hh"
#include "G4Positron.hh"
#include "G4RandomDirection.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VParticleChange.hh"
#include "Randomize.hh"

#include "RMGLog.hh"

RMGInnerBremsstrahlungProcess::RMGInnerBremsstrahlungProcess(
    const G4String& aNamePrefix,
    G4ProcessType aType
)
    : G4WrapperProcess(aNamePrefix, aType) {
  this->DefineCommands();
}

G4VParticleChange* RMGInnerBremsstrahlungProcess::AtRestDoIt(const G4Track& aTrack, const G4Step& aStep) {
  auto particle_change = pRegProcess->AtRestDoIt(aTrack, aStep);

  // If IB is disabled or no secondaries produced, return unchanged
  if (!fEnabled || particle_change->GetNumberOfSecondaries() == 0) { return particle_change; }

  RMGLog::OutFormat(RMGLog::debug_event, "{}: Processing decay at rest", GetProcessName());

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

  RMGLog::OutFormat(RMGLog::debug_event, "{}: Processing decay", GetProcessName());

  // Generate Inner Bremsstrahlung for any beta electrons in the secondaries
  GenerateInnerBremsstrahlungForSecondaries(particle_change, aTrack);

  return particle_change;
}

void RMGInnerBremsstrahlungProcess::GenerateInnerBremsstrahlungForSecondaries(
    G4VParticleChange* particle_change,
    const G4Track& parent_track
) {

  std::vector<double> omegas, cdf;

  // Snapshot the count before appending secondaries below.
  const int num_secondaries = particle_change->GetNumberOfSecondaries();

  // Loop through all secondaries to find beta electrons
  for (int i = 0; i < num_secondaries; i++) {
    auto secondary_track = particle_change->GetSecondary(i);

    if (!IsBetaElectron(secondary_track)) continue;

    auto electron_energy = secondary_track->GetKineticEnergy();
    if (electron_energy < 0.0 * CLHEP::keV) continue;

    // Compute the IB spectrum once and reuse it for both the emission probability and the
    // photon-energy sampling.
    auto ib_probability = ComputeIBSpectrum(electron_energy, omegas, cdf) * fBiasingFactor;

    // the biasing factor is user-settable and unbounded, so the scaled probability can exceed 1.
    if (ib_probability > 1.0) {
      RMGLog::OutFormat(
          RMGLog::warning,
          "{}: IB probability {:.6f} exceeds 1 (biasing factor {:.3g}); clamping to 1",
          GetProcessName(),
          ib_probability,
          fBiasingFactor
      );
      ib_probability = 1.0;
    }

    RMGLog::OutFormat(
        RMGLog::debug_event,
        "{}: Beta electron energy: {:.3f} keV, IB probability: {:.6f}",
        GetProcessName(),
        electron_energy / CLHEP::keV,
        ib_probability
    );

    // Sample whether IB occurs
    if (G4UniformRand() >= ib_probability) continue;

    // Sample photon energy from IB spectrum
    auto gamma_energy = SamplePhotonEnergy(omegas, cdf);

    // Get position and time from the decay location
    auto position = secondary_track->GetPosition();
    auto time = secondary_track->GetGlobalTime();
    auto touchable = secondary_track->GetTouchableHandle();

    // Create the IB gamma ray as an additional secondary
    auto dyn_particle = new G4DynamicParticle(G4Gamma::Definition(), G4RandomDirection(), gamma_energy);
    auto ib_gamma_track = new G4Track(dyn_particle, time, position);
    ib_gamma_track->SetTouchableHandle(touchable);
    ib_gamma_track->SetParentID(parent_track.GetTrackID()); // Same parent as decay
    ib_gamma_track->SetTrackStatus(fAlive);

    // Add the IB gamma through as additional secondary.
    particle_change->AddSecondary(ib_gamma_track);

    RMGLog::OutFormat(
        RMGLog::debug_event,
        "{}: Generated IB photon {:.3f} keV from beta {:.3f} keV",
        GetProcessName(),
        gamma_energy / CLHEP::keV,
        electron_energy / CLHEP::keV
    );
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

double RMGInnerBremsstrahlungProcess::ComputeIBSpectrum(
    double electron_energy,
    std::vector<double>& omegas,
    std::vector<double>& cdf
) {
  omegas.clear();
  cdf.clear();

  // Convert electron energy to dimensionless units
  double W_prime = electron_energy / CLHEP::electron_mass_c2 + 1.0;
  if (W_prime <= 1.0) return 0.0;

  // Integration parameters
  const int num_points = 100;
  double max_omega = W_prime - 1.0 - 0.01; // Leave some margin
  if (max_omega <= 0.01) return 0.0;

  double delta_omega = max_omega / num_points;

  omegas.reserve(num_points);
  cdf.reserve(num_points);

  // Cumulative trapezoidal integral of the spectrum. cdf[i] holds the integral up to omegas[i],
  // so cdf.back() is the total emission probability, and the same array is directly usable for
  // inverse-CDF sampling in SamplePhotonEnergy().
  double total = 0.0;
  double prev_phi = 0.0;
  for (int i = 0; i < num_points; i++) {
    double omega = 0.01 + i * delta_omega;
    double phi = PhiFunction(W_prime, omega);
    if (i > 0) total += 0.5 * (prev_phi + phi) * delta_omega;
    omegas.push_back(omega);
    cdf.push_back(total);
    prev_phi = phi;
  }

  return total;
}

double RMGInnerBremsstrahlungProcess::SamplePhotonEnergy(
    const std::vector<double>& omegas,
    const std::vector<double>& cdf
) {
  double total = cdf.empty() ? 0.0 : cdf.back();
  if (total <= 0.0) return 0.01 * CLHEP::electron_mass_c2; // Fallback

  // Sample from the (normalized) CDF
  double r = G4UniformRand();
  for (size_t i = 0; i < cdf.size(); i++) {
    if (r <= cdf[i] / total) {
      // Convert back to energy
      return omegas[i] * CLHEP::electron_mass_c2;
    }
  }

  // Fallback to the last value
  return omegas.back() * CLHEP::electron_mass_c2;
}

void RMGInnerBremsstrahlungProcess::DefineCommands() {

  // One process instance is constructed per radioactive nucleus, but the biasing factor and its
  // messenger are shared. Register the messenger only once, otherwise hundreds of
  // messengers would be created at the same command path.
  static std::once_flag messenger_flag;
  std::call_once(messenger_flag, [] {
    fMessenger = std::make_unique<G4GenericMessenger>(
        nullptr,
        "/RMG/Processes/InnerBremsstrahlung/",
        "Commands for controlling the inner bremsstrahlung process"
    );

    fMessenger->DeclareProperty("BiasingFactor", fBiasingFactor)
        .SetGuidance("Sets a biasing factor for IB probability")
        .SetParameterName("factor", false)
        .SetStates(G4State_PreInit, G4State_Idle);
  });
}

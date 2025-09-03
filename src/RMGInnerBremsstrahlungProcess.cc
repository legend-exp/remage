#include "RMGInnerBremsstrahlungProcess.hh"

<<<<<<< HEAD
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4Gamma.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4VParticleChange.hh"
#include "G4DynamicParticle.hh"
#include "G4EventManager.hh"
#include "G4TrackingManager.hh"
#include "G4SteppingManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
=======
#include <algorithm>
#include <cmath>
#include <vector>

#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4EventManager.hh"
#include "G4Gamma.hh"
#include "G4PhysicalConstants.hh"
#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4VParticleChange.hh"
>>>>>>> fc59aa2a8cb05581992f54f59b411f9705c1ddc8
#include "Randomize.hh"

#include "RMGLog.hh"

<<<<<<< HEAD
#include <cmath>
#include <vector>
#include <algorithm>

RMGInnerBremsstrahlungProcess::RMGInnerBremsstrahlungProcess(const G4String& aNamePrefix, G4ProcessType aType) : G4WrapperProcess(aNamePrefix, aType)
{
    fAlpha = CLHEP::fine_structure_const;
    fElectronMass = CLHEP::electron_mass_c2;
    fEnabled = true;
    fIBProbabilityScale = 1.0;
    RMGLog::OutFormat(RMGLog::detail, "{}: Inner Bremsstrahlung wrapper process initialized",
                     GetProcessName());
}

G4VParticleChange* RMGInnerBremsstrahlungProcess::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
    auto particleChange = pRegProcess->PostStepDoIt(aTrack, aStep);

    
    // If IB is disabled or no secondaries produced, return unchanged
    if (!fEnabled || particleChange->GetNumberOfSecondaries() == 0) {
        return particleChange;
    }

    RMGLog::OutFormat(RMGLog::debug,
                     "{}: Processing decay with {} secondaries",
                     GetProcessName(), particleChange->GetNumberOfSecondaries());

    // Generate Inner Bremsstrahlung for any beta electrons in the secondaries
    GenerateInnerBremsstrahlungForSecondaries(particleChange, aTrack, aStep);

    return particleChange;
}

void RMGInnerBremsstrahlungProcess::GenerateInnerBremsstrahlungForSecondaries(G4VParticleChange* particleChange, const G4Track& parentTrack, const G4Step& aStep) {

    // First check if this decay actually produces electrons/positrons (beta decay)
    bool hasBetaParticles = false;
    for (G4int i = 0; i < particleChange->GetNumberOfSecondaries(); i++) {
        G4Track* track = particleChange->GetSecondary(i);
        if (track->GetDefinition() == G4Electron::Definition() ||
            track->GetDefinition() == G4Positron::Definition()) {
            hasBetaParticles = true;
            break;
        }
    }

    if (!hasBetaParticles) {
        RMGLog::OutFormat(RMGLog::debug, "{}: No beta particles found, skipping IB generation",
                         GetProcessName());
        return;
    }

    // Loop through all secondaries to find beta electrons
    for (G4int i = 0; i < particleChange->GetNumberOfSecondaries(); i++) {
        G4Track* secondaryTrack = particleChange->GetSecondary(i);

        if (IsBetaElectron(secondaryTrack)) {
            G4double electronEnergy = secondaryTrack->GetKineticEnergy();
            if (electronEnergy < 0.0 * CLHEP::keV) {
                continue;
            }

            // Calculate IB probability (with optional scaling)
            G4double ibProbability = CalculateIBProbability(electronEnergy) * fIBProbabilityScale;

            RMGLog::OutFormat(RMGLog::debug,
                             "{}: Beta electron energy: {:.3f} keV, IB probability: {:.6f}",
                             GetProcessName(), electronEnergy/keV, ibProbability);

            // Sample whether IB occurs
            if (G4UniformRand() < ibProbability) {
                // Sample photon energy from IB spectrum
                G4double gammaEnergy = SamplePhotonEnergy(electronEnergy);

                // Generate random direction for the gamma (isotropic)
                G4double cosTheta = 2.0 * G4UniformRand() - 1.0;
                G4double sinTheta = std::sqrt(1.0 - cosTheta*cosTheta);
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
                G4DynamicParticle* dynParticle = new G4DynamicParticle(G4Gamma::Definition(),
                                                                      direction, gammaEnergy);
                G4Track* ibGammaTrack = new G4Track(dynParticle, time, position);
                ibGammaTrack->SetTouchableHandle(touchable);
                ibGammaTrack->SetParentID(parentTrack.GetTrackID());  // Same parent as decay
                ibGammaTrack->SetTrackStatus(fAlive);

                // Add the IB gamma to the stepping manager's secondary stack
                G4SteppingManager* steppingManager = G4EventManager::GetEventManager()
                    ->GetTrackingManager()->GetSteppingManager();
                steppingManager->GetfSecondary()->push_back(ibGammaTrack);

                RMGLog::OutFormat(RMGLog::debug,
                                 "{}: Generated IB photon {:.3f} keV from beta {:.3f} keV",
                                 GetProcessName(), gammaEnergy/keV, electronEnergy/keV);
            }
        }
    }
}

G4bool RMGInnerBremsstrahlungProcess::IsBetaElectron(G4Track* track) {
    // Check if this secondary is an electron or positron (beta particles)
    return (track->GetDefinition() == G4Electron::Definition() ||
            track->GetDefinition() == G4Positron::Definition());
}

G4double RMGInnerBremsstrahlungProcess::PhiFunction(G4double W_prime, G4double omega) {
    // Ensure W is physically meaningful
    G4double W = W_prime - omega;
    if (W <= 1.0) // Below rest mass energy
        return 0.0;

    G4double p = std::sqrt(W*W - 1.0); // Momentum of electron after photon emission
    G4double p_prime = std::sqrt(W_prime*W_prime - 1.0); // Momentum before photon emission

    if (p_prime <= 0.0 || omega <= 0.0) return 0.0;

    G4double bracket_term = ((W*W + W_prime*W_prime)/(W_prime*p)) * std::log(W + p) - 2.0;

    G4double result = (fAlpha * p) / (pi * omega * p_prime) * bracket_term;
    return std::max(0.0, result); // Ensure non-negative results
}

G4double RMGInnerBremsstrahlungProcess::CalculateIBProbability(G4double electronEnergy) {
    // Convert electron energy to dimensionless units
    G4double W_prime = electronEnergy / fElectronMass + 1.0;

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

        G4double phi1 = PhiFunction(W_prime, omega1);
        G4double phi2 = PhiFunction(W_prime, omega2);

        totalProb += 0.5 * (phi1 + phi2) * deltaOmega;
    }

    return totalProb;
}

G4double RMGInnerBremsstrahlungProcess::SamplePhotonEnergy(G4double electronEnergy) {
    // Convert electron energy to dimensionless units
    G4double W_prime = electronEnergy / fElectronMass + 1.0;

    // Create a cumulative distribution function (CDF) for sampling
    const int numPoints = 100;
    G4double maxOmega = W_prime - 1.0 - 0.01; // Leave some margin

    if (maxOmega <= 0.01) return 0.01 * fElectronMass;

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
        for (size_t i = 0; i < cdf.size(); i++) {
            cdf[i] /= sum;
        }
    }
    else {
        return 0.01 * fElectronMass; // Fallback
    }

    // Sample from the CDF
    G4double r = G4UniformRand();
    for (size_t i = 0; i < cdf.size(); i++) {
        if (r <= cdf[i]) {
            // Convert back to energy in keV
            return omegas[i] * fElectronMass;
        }
    }

    // Fallback to the last value
    return omegas.back() * fElectronMass;
=======
RMGInnerBremsstrahlungProcess::RMGInnerBremsstrahlungProcess(
    const G4String& aNamePrefix,
    G4ProcessType aType
)
    : G4WrapperProcess(aNamePrefix, aType), fAlpha(1.0 / 137.0), fElectronMass(511.0 * keV),
      fEnabled(true), fIBProbabilityScale(1.0) {
  RMGLog::OutFormat(
      RMGLog::detail,
      "{}: Inner Bremsstrahlung wrapper process initialized",
      GetProcessName()
  );
}

G4VParticleChange* RMGInnerBremsstrahlungProcess::PostStepDoIt(
    const G4Track& aTrack,
    const G4Step& aStep
) {
  auto particleChange = pRegProcess->PostStepDoIt(aTrack, aStep);


  // If IB is disabled or no secondaries produced, return unchanged
  if (!fEnabled || particleChange->GetNumberOfSecondaries() == 0) { return particleChange; }

  RMGLog::OutFormat(
      RMGLog::debug,
      "{}: Processing decay with {} secondaries",
      GetProcessName(),
      particleChange->GetNumberOfSecondaries()
  );

  // Generate Inner Bremsstrahlung for any beta electrons in the secondaries
  GenerateInnerBremsstrahlungForSecondaries(particleChange, aTrack, aStep);

  return particleChange;
}

void RMGInnerBremsstrahlungProcess::GenerateInnerBremsstrahlungForSecondaries(
    G4VParticleChange* particleChange,
    const G4Track& parentTrack,
    const G4Step& aStep
) {

  // First check if this decay actually produces electrons/positrons (beta decay)
  bool hasBetaParticles = false;
  for (G4int i = 0; i < particleChange->GetNumberOfSecondaries(); i++) {
    G4Track* track = particleChange->GetSecondary(i);
    if (track->GetDefinition() == G4Electron::Definition() ||
        track->GetDefinition() == G4Positron::Definition()) {
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
      if (electronEnergy < 0.0 * keV) { continue; }

      // Calculate IB probability (with optional scaling)
      G4double ibProbability = CalculateIBProbability(electronEnergy) * fIBProbabilityScale;

      RMGLog::OutFormat(
          RMGLog::debug,
          "{}: Beta electron energy: {:.3f} keV, IB probability: {:.6f}",
          GetProcessName(),
          electronEnergy / keV,
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
        G4DynamicParticle* dynParticle = new G4DynamicParticle(
            G4Gamma::Definition(),
            direction,
            gammaEnergy
        );
        G4Track* ibGammaTrack = new G4Track(dynParticle, time, position);
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
            gammaEnergy / keV,
            electronEnergy / keV
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

  G4double result = (fAlpha * p) / (pi * omega * p_prime) * bracket_term;
  return std::max(0.0, result); // Ensure non-negative results
}

G4double RMGInnerBremsstrahlungProcess::CalculateIBProbability(G4double electronEnergy) {
  // Convert electron energy to dimensionless units
  G4double W_prime = electronEnergy / fElectronMass + 1.0;

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

    G4double phi1 = PhiFunction(W_prime, omega1);
    G4double phi2 = PhiFunction(W_prime, omega2);

    totalProb += 0.5 * (phi1 + phi2) * deltaOmega;
  }

  return totalProb;
}

G4double RMGInnerBremsstrahlungProcess::SamplePhotonEnergy(G4double electronEnergy) {
  // Convert electron energy to dimensionless units
  G4double W_prime = electronEnergy / fElectronMass + 1.0;

  // Create a cumulative distribution function (CDF) for sampling
  const int numPoints = 100;
  G4double maxOmega = W_prime - 1.0 - 0.01; // Leave some margin

  if (maxOmega <= 0.01) return 0.01 * fElectronMass;

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
    for (size_t i = 0; i < cdf.size(); i++) { cdf[i] /= sum; }
  } else {
    return 0.01 * fElectronMass; // Fallback
  }

  // Sample from the CDF
  G4double r = G4UniformRand();
  for (size_t i = 0; i < cdf.size(); i++) {
    if (r <= cdf[i]) {
      // Convert back to energy in keV
      return omegas[i] * fElectronMass;
    }
  }

  // Fallback to the last value
  return omegas.back() * fElectronMass;
>>>>>>> fc59aa2a8cb05581992f54f59b411f9705c1ddc8
}

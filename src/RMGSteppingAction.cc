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

#include "RMGSteppingAction.hh"

#include "G4GenericMessenger.hh"
#include "G4Ions.hh"
#include "G4Step.hh"
#include "G4Threading.hh"

#include "RMGEventAction.hh"
#include "RMGLog.hh"

RMGSteppingAction::RMGSteppingAction(RMGEventAction*) { this->DefineCommands(); }

void RMGSteppingAction::UserSteppingAction(const G4Step* step) {

  if (fSkipTracking) {
    step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
    return;
  }

  // Kill _daughter_ nuclei with a lifetime longer than a user-defined threshold. This applies to
  // the defined half-life of the particle, and not the sampled time to the decay of the secondary
  // nucleus.
  if (fDaughterKillLifetime > 0) {
    auto track = step->GetTrack();
    auto particle_type = track->GetDefinition()->GetParticleType();

    if (particle_type == "nucleus" && track->GetParentID() > 0 &&
        track->GetKineticEnergy() < 0.1 * CLHEP::keV) {
      auto ion = dynamic_cast<G4Ions*>(track->GetDefinition());
      double lifetime = ion->GetPDGLifeTime();
      double excitation = ion->GetExcitationEnergy();

      // inspired by G4RadioactiveDecay::IsApplicable
      if (lifetime > fDaughterKillLifetime && excitation <= 0) {
        RMGLog::OutFormat(
            RMGLog::debug,
            "Killing daughter nucleus {} (lifetime={} us)",
            ion->GetParticleName(),
            lifetime / CLHEP::us
        );

        // note: UserSteppingAction is not called after step 0. Secondaries might be generated if
        // RadioactiveDecay takes place at step 1.
        auto new_status = track->GetCurrentStepNumber() > 1 ? fStopAndKill : fKillTrackAndSecondaries;
        track->SetTrackStatus(new_status);
      }
    }
  }
}

void RMGSteppingAction::SetDaughterKillLifetime(double max_lifetime) {

  fDaughterKillLifetime = max_lifetime;
  if (fDaughterKillLifetime > 0 && G4Threading::IsMasterThread()) {
    RMGLog::OutFormat(
        RMGLog::summary,
        "Enabled killing of daughter nuclei with a lifetime longer than {} us.",
        fDaughterKillLifetime / CLHEP::us
    );
  }
}

void RMGSteppingAction::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Processes/Stepping/",
      "Commands for controlling physics processes"
  );

  fMessenger
      ->DeclareMethodWithUnit("DaughterNucleusMaxLifetime", "us", &RMGSteppingAction::SetDaughterKillLifetime)
      .SetGuidance(
          "Determines which unstable daughter nuclei will be killed, if they are at rest, "
          "depending on their lifetime."
      )
      .SetGuidance(
          "This applies to the defined lifetime of the nucleus, and not on the sampled "
          "actual halflife of the simulated particle."
      )
      .SetGuidance("Set to -1 to disable this feature.")
      .SetParameterName("max_lifetime", false)
      .SetDefaultValue("-1")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("SkipTracking", fSkipTracking)
      .SetGuidance("Immediately discard tracks after primary particle generation. This feature is meant for debugging primary generation.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);
}


// vim: tabstop=2 shiftwidth=2 expandtab

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

#include "RMGParticleFilterOutputScheme.hh"

#include <set>

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"

#include "RMGLog.hh"

RMGParticleFilterOutputScheme::RMGParticleFilterOutputScheme() { this->DefineCommands(); }


std::optional<G4ClassificationOfNewTrack> RMGParticleFilterOutputScheme::
    StackingActionClassify(const G4Track* aTrack, int stage) {

  int pdg = aTrack->GetDefinition()->GetPDGEncoding();
  if (fParticles.find(pdg) == fParticles.end()) return std::nullopt;

  RMGLog::OutDev(RMGLog::debug, "Filtering out particle with PDG code ", pdg,
      " in RMGParticleFilterOutputScheme");
  return fKill;
}

void RMGParticleFilterOutputScheme::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Output/ParticleFilter/",
      "Commands for filtering particles out by PDG encoding.");

  fMessenger->DeclareMethod("AddParticle", &RMGParticleFilterOutputScheme::AddParticle)
      .SetGuidance("Add a particle to be filtered out by its PDG code. User is responsible for "
                   "correct PDG code.")
      .SetParameterName(0, "PDGcode", false, false)
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

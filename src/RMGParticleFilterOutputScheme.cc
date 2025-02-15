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

#include "RMGLog.hh"

RMGParticleFilterOutputScheme::RMGParticleFilterOutputScheme() { this->DefineCommands(); }

void RMGParticleFilterOutputScheme::AddKeepVolume(std::string name) {
  if (!fKillVolumes.empty()) {
    RMGLog::OutDev(RMGLog::error, "Conflicting requests for kill/keep volume in ParticleFilter. "
                                  "Trying to assign keep-volume but a kill-volume already exists.");
  }

  fKeepVolumes.insert(name);
}

void RMGParticleFilterOutputScheme::AddKillVolume(std::string name) {
  if (!fKeepVolumes.empty()) {
    RMGLog::OutDev(RMGLog::error, "Conflicting requests for kill/keep volume in ParticleFilter. "
                                  "Trying to assign kill-volume but a keep-volume already exists.");
  }

  fKillVolumes.insert(name);
}

std::optional<G4ClassificationOfNewTrack> RMGParticleFilterOutputScheme::
    StackingActionClassify(const G4Track* aTrack, int stage) {

  const int pdg = aTrack->GetDefinition()->GetPDGEncoding();
  // If the particle is not marked to kill, let it go
  if (fParticles.find(pdg) == fParticles.end()) return std::nullopt;

  const auto pv = aTrack->GetTouchableHandle()->GetVolume();
  // I am not 100% sure the physical volume will exist at this point. Can remove after some tests
  if (!pv) { RMGLog::OutDev(RMGLog::debug, "Physical volume does not exist in ParticleFilter!"); }
  const auto pv_name = pv->GetName();
  // If a kill volume is specified only kill if in the kill volume.
  if (!fKillVolumes.empty() && fKillVolumes.find(pv_name) == fKillVolumes.end())
    return std::nullopt;
  // If a keep volume is specified only keep particle if in that volume.
  if (fKeepVolumes.find(pv_name) != fKeepVolumes.end()) return std::nullopt;

  // If this is the primary particle we can not kill it without crashing the simulation
  if (aTrack->GetTrackID() == 0) {
    RMGLog::OutDev(RMGLog::warning,
        "ParticleFilter: The primary particle was specified to kill. "
        "This would cause the simulation to crash... so simulating it anyways. ");
    return std::nullopt;
  }

  // We land here if
  // i) Particle is marked to kill.
  // ii) No Kill volume specified or the particle is in the kill volume.
  // iii) Particle is not in the keep volume.
  // iiii) Particle is not the primary particle.
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

  fMessenger->DeclareMethod("AddKeepVolume", &RMGParticleFilterOutputScheme::AddKeepVolume)
      .SetGuidance("Add a physical volume by name in which all specified Particles will be kept. "
                   "They will be killed everywhere else. Can NOT be mixed with KillVolumes.")
      .SetParameterName(0, "PhysicalVolumeName", false, false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("AddKillVolume", &RMGParticleFilterOutputScheme::AddKillVolume)
      .SetGuidance("Add a physical volume by name in which all specified Particles will be killed. "
                   "They will only be killed in this volume. Can NOT be mixed with KeepVolumes.")
      .SetParameterName(0, "PhysicalVolumeName", false, false)
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

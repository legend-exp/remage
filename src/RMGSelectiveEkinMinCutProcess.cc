// Copyright (C) 2026
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.

#include "RMGSelectiveEkinMinCutProcess.hh"

#include <limits>

#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"

#include "RMGHardware.hh"

RMGSelectiveEkinMinCutProcess::RMGSelectiveEkinMinCutProcess(const G4String& process_name)
    : G4UserSpecialCuts(process_name) {}

G4double RMGSelectiveEkinMinCutProcess::PostStepGetPhysicalInteractionLength(
    const G4Track& aTrack,
    G4double previousStepSize,
    G4ForceCondition* condition
) {
  *condition = NotForced;

  const auto* volume = aTrack.GetVolume();
  if (!volume) return std::numeric_limits<G4double>::max();

  const auto* logical = volume->GetLogicalVolume();
  const auto& particle_name = aTrack.GetParticleDefinition()->GetParticleName();
  if (!RMGHardware::IsEminLimitParticleSelected(logical, particle_name)) {
    return std::numeric_limits<G4double>::max();
  }

  return G4UserSpecialCuts::PostStepGetPhysicalInteractionLength(aTrack, previousStepSize, condition);
}

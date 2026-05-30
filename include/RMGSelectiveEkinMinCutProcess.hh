// Copyright (C) 2026
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.

#ifndef _RMG_SELECTIVE_EKIN_MIN_CUT_PROCESS_HH_
#define _RMG_SELECTIVE_EKIN_MIN_CUT_PROCESS_HH_

#include "G4UserSpecialCuts.hh"

/**
 * @brief Variant of @c G4UserSpecialCuts that only enforces the kinetic-energy cut on the
 * (logical-volume, particle) pairs registered with @ref RMGHardware.
 *
 * Where no selective cut applies, the process returns an infinite interaction length, so it
 * is effectively transparent. This avoids the cost of applying the standard
 * @c G4UserSpecialCuts globally when the cut is only desired in a few volumes.
 */
class RMGSelectiveEkinMinCutProcess : public G4UserSpecialCuts {

  public:

    explicit RMGSelectiveEkinMinCutProcess(const G4String& process_name = "UserSpecialCut");
    ~RMGSelectiveEkinMinCutProcess() override = default;

    /**
     * @brief Defer to the base class only when the volume/particle pair is registered for
     * a selective cut; otherwise return an infinite interaction length.
     */
    G4double PostStepGetPhysicalInteractionLength(
        const G4Track& aTrack,
        G4double previousStepSize,
        G4ForceCondition* condition
    ) override;
};

#endif

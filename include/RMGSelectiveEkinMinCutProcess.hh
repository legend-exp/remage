// Copyright (C) 2026
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.

#ifndef _RMG_SELECTIVE_EKIN_MIN_CUT_PROCESS_HH_
#define _RMG_SELECTIVE_EKIN_MIN_CUT_PROCESS_HH_

#include "G4UserSpecialCuts.hh"

class RMGSelectiveEkinMinCutProcess : public G4UserSpecialCuts {

  public:

    explicit RMGSelectiveEkinMinCutProcess(const G4String& process_name = "UserSpecialCut");
    ~RMGSelectiveEkinMinCutProcess() override = default;

    G4double PostStepGetPhysicalInteractionLength(
        const G4Track& aTrack,
        G4double previousStepSize,
        G4ForceCondition* condition
    ) override;
};

#endif

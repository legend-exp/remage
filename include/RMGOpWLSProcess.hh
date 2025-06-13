// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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


#ifndef _RMG_OP_WLS_PROCESS_
#define _RMG_OP_WLS_PROCESS_

#include "G4ParticleDefinition.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VParticleChange.hh"
#include "G4WrapperProcess.hh"
#include "globals.hh"

class RMGOpWLSProcess : public G4WrapperProcess {

  public:

    explicit RMGOpWLSProcess(const G4String& aNamePrefix = "RMG", G4ProcessType aType = fOptical);

    virtual ~RMGOpWLSProcess() = default;

    G4VParticleChange* PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) override;

    void BuildPhysicsTable(const G4ParticleDefinition& aParticleType) override;
};
#endif

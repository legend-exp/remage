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


#ifndef _RMG_NEUTRON_CAPTURE_PROCESS_HH_
#define _RMG_NEUTRON_CAPTURE_PROCESS_HH_

#include "G4HadronicProcess.hh"
#include "globals.hh"

class RMGNeutronCaptureProcess : public G4HadronicProcess {
  public:

    explicit RMGNeutronCaptureProcess(const G4String& processName = "RMGnCapture");

    virtual ~RMGNeutronCaptureProcess();

    G4bool IsApplicable(const G4ParticleDefinition& aParticleType) final;

    // Override PostStepDoIt from G4HadronicProcess
    G4VParticleChange* PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) final;
};
#endif

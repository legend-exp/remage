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

#ifndef _RMGVGENERATOR_HH_
#define _RMGVGENERATOR_HH_

#include <memory>

#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"
#include "globals.hh"

class G4Event;
class G4Run;
class RMGVGenerator {

  public:

    RMGVGenerator() = delete;

    inline RMGVGenerator(std::string name) : fGeneratorName(name){};

    virtual inline ~RMGVGenerator() = default;

    RMGVGenerator(RMGVGenerator const&) = delete;
    RMGVGenerator& operator=(RMGVGenerator const&) = delete;
    RMGVGenerator(RMGVGenerator&&) = delete;
    RMGVGenerator& operator=(RMGVGenerator&&) = delete;

    virtual inline void BeginOfRunAction(const G4Run*) {};
    virtual inline void EndOfRunAction(const G4Run*) {};

    virtual void SetParticlePosition(G4ThreeVector vec) = 0;
    virtual void GeneratePrimaries(G4Event*) = 0;

    inline void SetReportingFrequency(int freq) { fReportingFrequency = freq; }
    inline std::string GetGeneratorName() { return fGeneratorName; }

  protected:

    std::string fGeneratorName;
    std::unique_ptr<G4UImessenger> fMessenger;
    int fReportingFrequency = 1000;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

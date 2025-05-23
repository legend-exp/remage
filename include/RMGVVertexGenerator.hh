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

#ifndef _RMG_V_VERTEX_GENERARTOR_HH_
#define _RMG_V_VERTEX_GENERARTOR_HH_

#include "RMGConfig.hh"
#if RMG_HAS_BXDECAY0
#include "bxdecay0_g4/vertex_generator_interface.hh"
#endif

#include <memory>

#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"

class G4Run;
#if RMG_HAS_BXDECAY0
class RMGVVertexGenerator : public bxdecay0_g4::VertexGeneratorInterface {
#else
class RMGVVertexGenerator {
#endif

  public:

    RMGVVertexGenerator(std::string name) : fGeneratorName(name) {}

    virtual ~RMGVVertexGenerator() = default;

    RMGVVertexGenerator(RMGVVertexGenerator const&) = delete;
    RMGVVertexGenerator& operator=(RMGVVertexGenerator const&) = delete;
    RMGVVertexGenerator(RMGVVertexGenerator&&) = delete;
    RMGVVertexGenerator& operator=(RMGVVertexGenerator&&) = delete;

    virtual void BeginOfRunAction(const G4Run*) {};
    virtual void EndOfRunAction(const G4Run*) {};

    virtual bool GenerateVertex(G4ThreeVector& v) {
      v = kDummyPrimaryPosition;
      return false;
    }
    void SetMaxAttempts(int val) { fMaxAttempts = val; }
    [[nodiscard]] int GetMaxAttempts() const { return fMaxAttempts; }

#if RMG_HAS_BXDECAY0
    void ShootVertex(G4ThreeVector& v) override { GenerateVertex(v); }
#endif

  protected:

    std::string fGeneratorName;
    int fMaxAttempts = 100;
    const G4ThreeVector kDummyPrimaryPosition = G4ThreeVector(0, 0, 0);

    std::unique_ptr<G4UImessenger> fMessenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

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

#ifndef _RMG_GENERATOR_DECAY0_HH_
#define _RMG_GENERATOR_DECAY0_HH_

#include <memory>

#include "G4ThreeVector.hh"

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

namespace bxdecay0_g4 {
  class PrimaryGeneratorAction;
}

class G4Event;
class RMGGeneratorDecay0 : public RMGVGenerator {

  public:

    RMGGeneratorDecay0(RMGVVertexGenerator* prim_gen);
    RMGGeneratorDecay0() = delete;
    ~RMGGeneratorDecay0();

    RMGGeneratorDecay0(RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0(RMGGeneratorDecay0&&) = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0&&) = delete;

    void GeneratePrimaries(G4Event*) override;
    inline void SetParticlePosition(G4ThreeVector) override{};

  private:

    std::unique_ptr<bxdecay0_g4::PrimaryGeneratorAction> fDecay0G4Generator;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

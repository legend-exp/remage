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

#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

namespace bxdecay0_g4 {
  class PrimaryGeneratorAction;
}

class G4Event;
class RMGGeneratorDecay0 : public RMGVGenerator {

  public:

    // This matches the BxDecay0 numbering, shifted by 1 as BxDecay0 starts counting at 1.
    enum class Process {
      k0vbb,           // neutrinoless double beta decay
      k0vbb_lambda_0,  // 0+ -> 0+ {2n} with RHC lambda
      k0vbb_lambda_02, // 0+ -> 0+, 2+ {N*} with RHC lambda
      k2vbb,           // 2 neutrino double beta decay
      k0vbb_M1,        // 0+ -> 0+ {2n} (Majoron, SI=1)
      k0vbb_M2,        // 0+ -> 0+ {2n} (Majoron, SI=2)
      k0vbb_M3,        // 0+ -> 0+ {2n} (Majoron, SI=3)
      k0vbb_M7,        // 0+ -> 0+ {2n} (Majoron, SI=7)
      k0vbb_lambda_2,  // 0+ -> 2+ {2n} with RHC lambda
      k2vbb_2,         // 0+ -> 2+ {2n}, {N*}
      k0vkb,           // EC + beta+  0+ -> 0+, 2+
      k2vkb,           // EC + beta+  0+ -> 0+, 2+
      k0v2k,           // double EC  0+ -> 0+, 2+
      k2v2k,           // double EC  0+ -> 0+, 2+
      k2vbb_bos0,      //  0+ -> 0+ with bosonic neutrinos
      k2vbb_bos2,      // 0+ -> 2+ with bosonic neutrinos
      k0vbb_eta_s,     // 0+ -> 0+ with RHC eta simplified expression
      k0vbb_eta_nmes,  //  0+ -> 0+ with RHC eta and specific NMEs
      k2vbb_lv,        // 0+ -> 0+ with Lorentz violation
      k0v4b            // 0+ -> 0+ Quadruple beta decay
    };

    RMGGeneratorDecay0(RMGVVertexGenerator* prim_gen);
    RMGGeneratorDecay0() = delete;
    ~RMGGeneratorDecay0();

    RMGGeneratorDecay0(RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0(RMGGeneratorDecay0&&) = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0&&) = delete;

    void GeneratePrimaries(G4Event*) override;
    void SetParticlePosition(G4ThreeVector) override{};

    void BeginOfRunAction(const G4Run*) override;
    inline void EndOfRunAction(const G4Run*) override {}

    void SetBackground(std::string);

    void SetUpdateSeeds(bool value) { fUpdateSeeds = value; }


  private:

    std::unique_ptr<bxdecay0_g4::PrimaryGeneratorAction> fDecay0G4Generator;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    bool fUpdateSeeds = false;

    // Nested messenger class
    class BxMessenger : public G4UImessenger {
      public:

        BxMessenger(RMGGeneratorDecay0* gen);
        ~BxMessenger();

        void SetNewValue(G4UIcommand* command, G4String newValues) override;

      private:

        RMGGeneratorDecay0* fGen;
        G4UIcommand* fGeneratorCmd;

        void GeneratorCmd(const std::string& parameters);
    };

    std::unique_ptr<BxMessenger> fUIMessenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

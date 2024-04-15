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

#ifndef _RMG_MASTER_GENERATOR_HH_
#define _RMG_MASTER_GENERATOR_HH_

#include <memory>

#include "G4VUserPrimaryGeneratorAction.hh"

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

class G4GenericMessenger;
class RMGMasterGenerator : public G4VUserPrimaryGeneratorAction {

  public:

    enum Confinement {
      kUnConfined,
      kVolume,
      kFromFile,
    };

    enum Generator {
      kG4gun,
      kGPS,
      kBxDecay0,
      kCosmicMuons,
      kUserDefined,
      kUndefined
    };

    RMGMasterGenerator();
    ~RMGMasterGenerator() = default;

    RMGMasterGenerator(RMGMasterGenerator const&) = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator const&) = delete;
    RMGMasterGenerator(RMGMasterGenerator&&) = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator&&) = delete;

    void GeneratePrimaries(G4Event* event) override;

    inline RMGVGenerator* GetGenerator() { return fGeneratorObj.get(); }
    inline RMGVVertexGenerator* GetVertexGenerator() { return fVertexGeneratorObj.get(); }
    [[nodiscard]] inline Confinement GetConfinement() const { return fConfinement; }

    void SetConfinement(Confinement code);
    void SetConfinementString(std::string code);
    void SetUserGenerator(RMGVGenerator* gen);
    void SetGenerator(Generator gen);
    void SetGeneratorString(std::string gen);

  private:

    Confinement fConfinement{RMGMasterGenerator::Confinement::kUnConfined};
    std::unique_ptr<RMGVVertexGenerator> fVertexGeneratorObj;

    Generator fGenerator{RMGMasterGenerator::Generator::kUndefined};
    std::unique_ptr<RMGVGenerator> fGeneratorObj;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

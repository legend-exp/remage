// Copyright (C) 2024 Manuel Huber
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

#ifndef _RMG_GENERATOR_FROM_FILE_HH_
#define _RMG_GENERATOR_FROM_FILE_HH_

#include <memory>

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"

#include "RMGAnalysisReader.hh"
#include "RMGVGenerator.hh"

namespace u = CLHEP;

class G4Event;
class RMGGeneratorFromFile : public RMGVGenerator {

  public:

    RMGGeneratorFromFile();
    ~RMGGeneratorFromFile() = default;

    RMGGeneratorFromFile(RMGGeneratorFromFile const&) = delete;
    RMGGeneratorFromFile& operator=(RMGGeneratorFromFile const&) = delete;
    RMGGeneratorFromFile(RMGGeneratorFromFile&&) = delete;
    RMGGeneratorFromFile& operator=(RMGGeneratorFromFile&&) = delete;

    void GeneratePrimaries(G4Event* event) override;
    void SetParticlePosition(G4ThreeVector pos) override { fParticlePosition = pos; }

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void OpenFile(std::string& name);

  private:

    struct RowData {
        int fG4Pid = -1;
        double fEkin = NAN;
        double fPx = NAN;
        double fPy = NAN;
        double fPz = NAN;
        RowData() {}; // NOLINT(modernize-use-equals-default)

        [[nodiscard]] bool IsValid() const {
          return fG4Pid != -1 && !std::isnan(fEkin) && !std::isnan(fPx) && !std::isnan(fPy) &&
                 !std::isnan(fPz);
        }
    };

    static RMGAnalysisReader* fReader;
    inline static RowData fRowData{};

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    std::string fNtupleDirectoryName = "vtx";

    std::unique_ptr<G4ParticleGun> fGun = nullptr;

    G4ThreeVector fParticlePosition;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

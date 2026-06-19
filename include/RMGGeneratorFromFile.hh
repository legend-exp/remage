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
/**
 * @brief Primary generator reading particle kinematics row-by-row from an ntuple file.
 *
 * Each row of the input ntuple (LH5/HDF5/ROOT, opened through @ref RMGAnalysisReader)
 * specifies a Geant4 PDG id, kinetic energy, momentum direction and global time. The vertex
 * position is supplied externally by the configured vertex generator via
 * @ref SetParticlePosition. The @c fNpart column allows several consecutive rows to be
 * combined into a single multi-particle primary vertex.
 */
class RMGGeneratorFromFile : public RMGVGenerator {

  public:

    RMGGeneratorFromFile();
    ~RMGGeneratorFromFile() = default;

    RMGGeneratorFromFile(RMGGeneratorFromFile const&) = delete;
    RMGGeneratorFromFile& operator=(RMGGeneratorFromFile const&) = delete;
    RMGGeneratorFromFile(RMGGeneratorFromFile&&) = delete;
    RMGGeneratorFromFile& operator=(RMGGeneratorFromFile&&) = delete;

    /** @brief Read the next row(s) from the input ntuple and create the primary vertex. */
    void GeneratePrimaries(G4Event*) override;
    /** @brief Set the vertex position for the next call to @ref GeneratePrimaries. */
    void SetParticlePosition(G4ThreeVector pos) override { fParticlePosition = pos; }

    /** @brief Open the input file and bind the row reader to the configured ntuple columns. */
    void BeginOfRunAction(const G4Run*) override;
    /** @brief Close the input file. */
    void EndOfRunAction(const G4Run*) override;

    /** @brief Set the path of the input ntuple file. */
    void OpenFile(std::string& name);

  private:

    struct RowData {
        int fG4Pid = -1;
        double fEkin = NAN;
        double fPx = NAN;
        double fPy = NAN;
        double fPz = NAN;
        double fTime = NAN;
        int fNpart = -1;
        double fXpos = NAN;
        double fYpos = NAN;
        double fZpos = NAN;
        RowData() {}; // NOLINT(modernize-use-equals-default)

        [[nodiscard]] bool IsValid(bool include_pos) const {
          bool kin_valid = fG4Pid != -1 && !std::isnan(fEkin) && !std::isnan(fPx) &&
                           !std::isnan(fPy) && !std::isnan(fPz) && !std::isnan(fTime) &&
                           fNpart != -1;
          bool pos_valid = !include_pos ||
                           (!std::isnan(fXpos) && !std::isnan(fYpos) && !std::isnan(fZpos));
          return kin_valid && pos_valid;
        }
    };

    static RMGAnalysisReader* fReader;
    inline static RowData fRowData{};

    bool fIncludePosition = false;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    std::string fNtupleDirectoryName = "vtx";

    std::unique_ptr<G4ParticleGun> fGun = nullptr;

    G4ThreeVector fParticlePosition;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

// Copyright (C) 2024 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
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


#ifndef _RMG_GENERATOR_MUSUN_COSMIC_MUONS_HH_
#define _RMG_GENERATOR_MUSUN_COSMIC_MUONS_HH_

#include <filesystem>

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"

#include "RMGAnalysisReader.hh"
#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

namespace u = CLHEP;

/**
 * @brief Row schema for the MUSUN input ntuple consumed by @ref RMGGeneratorMUSUNCosmicMuons.
 *
 * Position is in Geant4 length units; energy in MeV; angles in radians. Momentum is provided
 * either in Cartesian (@c fPx, @c fPy, @c fPz) or in spherical (@c fTheta, @c fPhi) form.
 */
struct RMGGeneratorMUSUNCosmicMuons_Data {
    G4int fID;
    G4int fType;
    G4double fEkin;
    G4double fX;
    G4double fY;
    G4double fZ;
    G4double fTheta;
    G4double fPhi;
    G4double fPx;
    G4double fPy;
    G4double fPz;
};


class G4Event;
/**
 * @brief Primary generator reading pre-sampled cosmic-muon kinematics from a MUSUN file.
 *
 * The input file (an ASCII MUSUN dump) is converted to a temporary Geant4 CSV ntuple at the
 * beginning of the run and read row-by-row by @ref RMGAnalysisReader. Vertex sampling is
 * controlled by the input file, so @ref SetParticlePosition is a no-op.
 */
class RMGGeneratorMUSUNCosmicMuons : public RMGVGenerator {

  public:

    RMGGeneratorMUSUNCosmicMuons();
    ~RMGGeneratorMUSUNCosmicMuons() = default;

    RMGGeneratorMUSUNCosmicMuons(RMGGeneratorMUSUNCosmicMuons const&) = delete;
    RMGGeneratorMUSUNCosmicMuons& operator=(RMGGeneratorMUSUNCosmicMuons const&) = delete;
    RMGGeneratorMUSUNCosmicMuons(RMGGeneratorMUSUNCosmicMuons&&) = delete;
    RMGGeneratorMUSUNCosmicMuons& operator=(RMGGeneratorMUSUNCosmicMuons&&) = delete;

    /** @brief Read the next muon entry from the temporary file and shoot it. */
    void GeneratePrimaries(G4Event*) override;
    /** @brief No-op: vertex sampling is fixed by the input MUSUN file. */
    void SetParticlePosition(G4ThreeVector) override{};

    /** @brief Convert the MUSUN ASCII input to Geant4 CSV and open it for reading. */
    void BeginOfRunAction(const G4Run*) override;
    /** @brief Close the input file and clean up the temporary directory. */
    void EndOfRunAction(const G4Run*) override;

  private:

    void DefineCommands();
    void SetMUSUNFile(G4String pathToFile);

    /**
     * @return True if the file contains cartesian momentum (fPx, fPy, fPz), or false for spherical
     * momentum (fPhi, fTheta).
     */
    bool PrepareCopy(std::string pathToFile);

    std::unique_ptr<G4ParticleGun> fGun = nullptr;
    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    G4String fPathToFile = "";
    std::filesystem::path fPathToTmpFolder;
    std::filesystem::path fPathToTmpFile;

    static RMGAnalysisReader* fAnalysisReader;
    static bool fHasCartesianMomentum;

    static RMGGeneratorMUSUNCosmicMuons_Data* fInputData;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

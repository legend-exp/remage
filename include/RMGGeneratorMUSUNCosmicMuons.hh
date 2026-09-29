// Copyright (C) 2024 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <https://www.gnu.org/licenses/>.


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
 * Raw file values follow native MUSUN output: position in cm, energy in GeV; angles in radians.
 * Momentum is provided either in Cartesian (@c fPx, @c fPy, @c fPz) or in spherical
 * (@c fTheta, @c fPhi) form.
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
 *
 * @details Both the @ref RMGAnalysisReader instance and the variables bound to the ntuple
 * columns have static storage duration, i.e. they are shared between all worker threads. The
 * consequences for parallel runs are:
 * - In a multithreaded run all threads pull from the same file cursor, serialized by the
 * reader mutex. Every row is still consumed exactly once, but the order in which the threads
 * reach the reader is not deterministic. The event id in the output therefore does not
 * correspond to the row index in the input, and the assignment is not reproducible from run
 * to run, not even with a fixed random seed.
 * - In a multiprocessing run every process is sequential and @ref BeginOfRunAction seeks over
 * the first @c p*N rows, with @c p the process number offset and @c N the number of events of
 * the run. The processes hence consume disjoint, contiguous blocks of the file, which must
 * hold at least @c K*N rows for @c K processes. Output event ids are offset by the same
 * @c p*N, so the correspondence between input row index and output event id is preserved.
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

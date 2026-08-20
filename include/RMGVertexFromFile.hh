// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

#ifndef _RMG_VERTEX_FROM_FILE_HH_
#define _RMG_VERTEX_FROM_FILE_HH_

#include <memory>
#include <string>

#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

#include "RMGAnalysisReader.hh"
#include "RMGVVertexGenerator.hh"

/**
 * @brief Vertex generator that reads positions sequentially from an ntuple file.
 *
 * Each row of the ntuple supplies one @c (x, y, z) triplet. Used to replay vertices
 * produced by an external sampler (e.g. an MC truth file) into a remage run.
 *
 * @details Both the @ref RMGAnalysisReader instance and the variables bound to the ntuple
 * columns have static storage duration, i.e. they are shared between all worker threads. The
 * consequences for parallel runs are:
 * - In a multithreaded run all threads pull from the same file cursor, serialized by the
 * reader mutex. Every row is still consumed exactly once, but the order in which the threads
 * reach the reader is not deterministic. The event id in the output therefore does not
 * correspond to the row index in the input, and the assignment is not reproducible from run
 * to run, not even with a fixed random seed. @ref RMGGeneratorFromFile holds a second,
 * independent reader instance, so vertices and kinematics read from two separate tables can
 * be paired inconsistently; the combined position/kinematics table does not suffer from this.
 * - In a multiprocessing run every process is sequential and @ref BeginOfRunAction seeks over
 * the first @c p*N rows, with @c p the process number offset and @c N the number of events of
 * the run. The processes hence consume disjoint, contiguous blocks of the file, which must
 * hold at least @c K*N rows for @c K processes. Output event ids are offset by the same
 * @c p*N, so the correspondence between input row index and output event id is preserved.
 */
class RMGVertexFromFile : public RMGVVertexGenerator {

  public:

    RMGVertexFromFile();
    ~RMGVertexFromFile() = default;

    RMGVertexFromFile(RMGVertexFromFile const&) = delete;
    RMGVertexFromFile& operator=(RMGVertexFromFile const&) = delete;
    RMGVertexFromFile(RMGVertexFromFile&&) = delete;
    RMGVertexFromFile& operator=(RMGVertexFromFile&&) = delete;

    /**
     * @brief Read the next position row from the file.
     * @return False if the input has been exhausted (which aborts the run gracefully).
     */
    bool GenerateVertex(G4ThreeVector&) override;

    /** @brief Open the input file and bind the position columns. */
    void BeginOfRunAction(const G4Run*) override;
    /** @brief Close the input file. */
    void EndOfRunAction(const G4Run*) override;

    /** @brief Set the path of the input ntuple file. */
    void OpenFile(std::string& name);

  private:

    static RMGAnalysisReader* fReader;

    inline static double fXpos = NAN, fYpos = NAN, fZpos = NAN;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    std::string fNtupleDirectoryName = "vtx";
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

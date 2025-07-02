// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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
/**
 * @brief Abstract base class for vertex generators.
 *
 * This class defines the interface for generating primary vertex positions in a simulation run.
 * It provides hooks for run initialization (begin/end) and for generating a vertex.
 * When BxDecay0 is available, it implements the @c bxdecay0::VertexGeneratorInterface.
 */
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

    /**
     * @brief Generate a primary vertex position.
     *
     * Fills the provided vector @p v with the generated primary vertex position.
     * If vertex generation is not implemented, @p v is set to a dummy position (0,0,0)
     * and the function returns false.
     *
     * @param v Reference to the @c G4ThreeVector to hold the generated vertex.
     * @return True if a valid vertex was generated, false otherwise.
     */
    virtual bool GenerateVertex(G4ThreeVector& v) {
      v = kDummyPrimaryPosition;
      return false;
    }
    /**
     * @brief Set the maximum number of attempts for vertex generation.
     *
     * This controls how many iterations the generator will perform before giving up.
     *
     * @param val The maximum number of attempts.
     */
    void SetMaxAttempts(int val) { fMaxAttempts = val; }
    /**
     * @brief Get the maximum number of attempts for vertex generation.
     *
     * @return The maximum number of attempts.
     */
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

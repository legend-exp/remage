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

#ifndef _RMG_V_GENERATOR_HH_
#define _RMG_V_GENERATOR_HH_

#include <memory>

#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"
#include "globals.hh"

class G4Event;
class G4Run;
/**
 * @brief Abstract base class for primary generators.
 *
 * This class defines the interface for generators that create the primary
 * particle(s) and set their vertex position(s) for a simulation event.
 */
class RMGVGenerator {

  public:

    RMGVGenerator() = delete;

    RMGVGenerator(std::string name) : fGeneratorName(name) {};

    virtual ~RMGVGenerator() = default;

    RMGVGenerator(RMGVGenerator const&) = delete;
    RMGVGenerator& operator=(RMGVGenerator const&) = delete;
    RMGVGenerator(RMGVGenerator&&) = delete;
    RMGVGenerator& operator=(RMGVGenerator&&) = delete;

    /**
     * @brief Called at the beginning of a run.
     *
     * Derived generators can perform any necessary initialization in this method.
     */
    virtual void BeginOfRunAction(const G4Run*) {};
    /**
     * @brief Called at the end of a run.
     *
     * Derived generators can perform any necessary finalization in this method.
     */
    virtual void EndOfRunAction(const G4Run*) {};

    /**
     * @brief Set the primary vertex position.
     *
     * This pure virtual method must be implemented by derived classes to set the
     * primary vertex position for the generator.
     *
     * @param vec The desired primary vertex position.
     */
    virtual void SetParticlePosition(G4ThreeVector vec) = 0;
    /**
     * @brief Generate primary particles for an event.
     *
     * Derived generators must implement this method to create primary particles
     * and add them to the given @c G4Event.
     *
     * @param event Pointer to the @c G4Event.
     */
    virtual void GeneratePrimaries(G4Event* event) = 0;

    void SetReportingFrequency(int freq) { fReportingFrequency = freq; }
    std::string GetGeneratorName() { return fGeneratorName; }

  protected:

    std::string fGeneratorName;
    std::unique_ptr<G4UImessenger> fMessenger;
    int fReportingFrequency = 1000;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

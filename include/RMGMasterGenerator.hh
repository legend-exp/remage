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

#ifndef _RMG_MASTER_GENERATOR_HH_
#define _RMG_MASTER_GENERATOR_HH_

#include <memory>

#include "G4VUserPrimaryGeneratorAction.hh"

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

class G4Event;
class G4GenericMessenger;
class RMGMasterGenerator : public G4VUserPrimaryGeneratorAction {

  public:

    /**
     * @brief Enumeration for specifying the primary vertex confinement strategy.
     */
    enum class Confinement {
      kUnConfined, ///< No confinement is applied here; the generator has the duty to sample a primary vertex.
      kVolume,  ///< The primary vertex is confined to a specific detector volume.
      kFromFile ///< The primary vertex is read from an external file.
    };

    /**
     * @brief Enumeration for selecting the primary generator mode.
     */
    enum class Generator {
      kG4gun,            ///< The standard Geant4 particle gun.
      kGPS,              ///< The Geant4 General Particle Source.
      kBxDecay0,         ///< The BxDecay0 generator for double beta decay processes.
      kFromFile,         ///< A generator that reads primary vertex data from an external file.
      kCosmicMuons,      ///< A simple cosmic muon generator.
      kMUSUNCosmicMuons, ///< The MUSUN-based cosmic muon generator.
      kUserDefined,      ///< A user-specified custom generator.
      kGeomBench,        ///< The benchmark generator.
      kUndefined         ///< Undefined generator mode.
    };

    RMGMasterGenerator();
    ~RMGMasterGenerator() = default;

    RMGMasterGenerator(RMGMasterGenerator const&) = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator const&) = delete;
    RMGMasterGenerator(RMGMasterGenerator&&) = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator&&) = delete;

    /**
     * @brief Generate primary vertices for the event.
     *
     * This method delegates the creation of primary vertices to the configured generator.
     * Depending on the selected generator mode (e.g. G4gun, GPS, BxDecay0, CosmicMuons, etc.),
     * it produces one or more primary vertices for the event.
     *
     * @param event Pointer to the @c G4Event to which the primary vertices will be added.
     */
    void GeneratePrimaries(G4Event* event) override;

    /**
     * @brief Get the current primary generator.
     *
     * @return Pointer to the configured @ref RMGVGenerator instance.
     */
    RMGVGenerator* GetGenerator() { return fGeneratorObj.get(); }
    /**
     * @brief Get the current vertex generator.
     *
     * @return Pointer to the configured @ref RMGVVertexGenerator instance.
     */
    RMGVVertexGenerator* GetVertexGenerator() { return fVertexGeneratorObj.get(); }
    /**
     * @brief Retrieve the current vertex confinement strategy.
     *
     * @return The currently selected Confinement mode (@c kUnConfined, @c
     * kVolume, or @c kFromFile).
     */
    [[nodiscard]] Confinement GetConfinement() const { return fConfinement; }

    /**
     * @brief Set the primary vertex confinement strategy.
     *
     * The confinement strategy determines how the primary vertex is generated, for example
     * whether to generate it from a detector volume or to load it from an input file.
     *
     * @param code The Confinement mode to set.
     */
    void SetConfinement(Confinement code);
    /**
     * @brief Set the vertex confinement strategy using a string.
     *
     * The provided string is converted to a Confinement enum value.
     *
     * @param code The string specifying the confinement mode (e.g., "kUnConfined", "kVolume", "kFromFile").
     */
    void SetConfinementString(std::string code);
    /**
     * @brief Set a user-defined primary generator.
     *
     * This method allows the registration of a custom generator.
     * The user-defined generator pointer is owned by the manager.
     *
     * @param gen Pointer to an instance of a custom @ref RMGVGenerator.
     */
    void SetUserGenerator(RMGVGenerator* gen);
    /**
     * @brief Select one of the built-in primary generator modes.
     *
     * This method sets the generator mode (e.g., @c kG4gun, @c kGPS, @c
     * kBxDecay0, @c kCosmicMuons, etc.)
     * to be used when generating primary vertices.
     *
     * @param gen The generator mode to use.
     */
    void SetGenerator(Generator gen);
    /**
     * @brief Set the primary generator mode using a string.
     *
     * The method converts the provided string into a Generator enum value.
     *
     * @param gen The string representing the generator type (e.g., "kG4gun", "kGPS",
     *            "kBxDecay0", "kCosmicMuons", "kMUSUNCosmicMuons", etc.).
     */
    void SetGeneratorString(std::string gen);

  private:

    Confinement fConfinement{Confinement::kUnConfined};
    std::unique_ptr<RMGVVertexGenerator> fVertexGeneratorObj;

    Generator fGenerator{Generator::kUndefined};
    std::unique_ptr<RMGVGenerator> fGeneratorObj;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

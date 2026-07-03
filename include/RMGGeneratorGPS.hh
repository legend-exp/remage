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

#ifndef _RMG_GENERATOR_GPS_HH_
#define _RMG_GENERATOR_GPS_HH_

#include <memory>

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ThreeVector.hh"

#include "RMGVGenerator.hh"

/**
 * @brief @ref RMGVGenerator wrapper around Geant4's General Particle Source.
 *
 * If a vertex position has been provided via @ref SetParticlePosition, the primary vertices
 * produced by the GPS are translated by it after generation. The GPS position configuration
 * will not be used in this case.
 */
class RMGGeneratorGPS : public RMGVGenerator {

  public:

    RMGGeneratorGPS() : RMGVGenerator("GPS") {
      fParticleSource = std::make_unique<G4GeneralParticleSource>();
    }

    ~RMGGeneratorGPS() = default;

    /** @brief Generate a primary vertex from the GPS, optionally overriding the vertex position. */
    void GeneratePrimaries(G4Event* event) override {
      // the GPS is inherently thread-unsafe: only one source can be used at a time, and all
      // threads share the same internal global state. do not mutate that shared state (e.g.
      // via SetCentreCoords) — instead translate the resulting vertices below.
      auto first_vertex = event->GetNumberOfPrimaryVertex();
      fParticleSource->GeneratePrimaryVertex(event);

      if (fVertexPositionSet) {
        auto n_vertex = event->GetNumberOfPrimaryVertex();
        for (auto i = first_vertex; i < n_vertex; i++) {
          auto vertex = event->GetPrimaryVertex(i);
          vertex->SetPosition(fVertexPosition.x(), fVertexPosition.y(), fVertexPosition.z());
        }
      }
    }

    /** @brief Translate every GPS primary vertex to @p vec for the next event. */
    void SetParticlePosition(G4ThreeVector vec) override {
      fVertexPosition = vec;
      fVertexPositionSet = true;
    }

  private:

    bool fVertexPositionSet = false;
    G4ThreeVector fVertexPosition;

    std::unique_ptr<G4GeneralParticleSource> fParticleSource = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

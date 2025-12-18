// Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#ifndef _RMG_VERTEX_FROM_POINT_HH_
#define _RMG_VERTEX_FROM_POINT_HH_

#include <memory>

#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

#include "RMGVVertexGenerator.hh"

class RMGVertexFromPoint : public RMGVVertexGenerator {

  public:

    RMGVertexFromPoint();
    ~RMGVertexFromPoint() = default;

    RMGVertexFromPoint(RMGVertexFromPoint const&) = delete;
    RMGVertexFromPoint& operator=(RMGVertexFromPoint const&) = delete;
    RMGVertexFromPoint(RMGVertexFromPoint&&) = delete;
    RMGVertexFromPoint& operator=(RMGVertexFromPoint&&) = delete;

    bool GenerateVertex(G4ThreeVector& vertex) override {
      vertex = fVertex;
      return true;
    };

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    G4ThreeVector fVertex = kDummyPrimaryPosition;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

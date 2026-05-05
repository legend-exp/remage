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

#include "RMGUserInit.hh"

#include "RMGGeomBenchOutputScheme.hh"
#include "RMGGeometryCheckOutputScheme.hh"
#include "RMGIsotopeFilterScheme.hh"
#include "RMGParticleFilterScheme.hh"
#include "RMGTrackOutputScheme.hh"
#include "RMGVertexOutputScheme.hh"
#include "RMGVolumeDistanceStacker.hh"

void RMGUserInit::RegisterDefaultOptionalOutputSchemes() {
  AddOptionalOutputScheme<RMGVertexOutputScheme>("Vertex");
  AddOptionalOutputScheme<RMGIsotopeFilterScheme>("IsotopeFilter");
  AddOptionalOutputScheme<RMGParticleFilterScheme>("ParticleFilter");
  AddOptionalOutputScheme<RMGTrackOutputScheme>("Track");
  AddOptionalOutputScheme<RMGGeometryCheckOutputScheme>("GeometryCheck");
  AddOptionalOutputScheme<RMGGeomBenchOutputScheme>("GeomBench");
  AddOptionalOutputScheme<RMGVolumeDistanceStacker>("VolumeStacker");
}

void RMGUserInit::ActivateOptionalOutputScheme(std::string name) {
  auto it = fOptionalOutputSchemes.find(name);
  if (it == fOptionalOutputSchemes.end()) {
    RMGLog::Out(RMGLog::fatal, "Optional output scheme '", name, "' not found!");
  }

  if (std::find(fActivatedOutputScheme.begin(), fActivatedOutputScheme.end(), name) !=
      fActivatedOutputScheme.end()) {
    RMGLog::Out(RMGLog::fatal, "Optional output scheme '", name, "' already activated!");
  }

  fOutputSchemes.emplace_back((*it).second);
  fActivatedOutputScheme.emplace_back(name);
}

bool RMGUserInit::IsOptionalOutputSchemeActivated(std::string name) {
  return std::find(fActivatedOutputScheme.begin(), fActivatedOutputScheme.end(), name) !=
         fActivatedOutputScheme.end();
}

// vim: tabstop=2 shiftwidth=2 expandtab

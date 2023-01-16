// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
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

#include "RMGGeneratorDecay0.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

#include "ProjectInfo.hh"
#if RMG_HAS_BXDECAY0
#include "bxdecay0_g4/primary_generator_action.hh"
#endif

RMGGeneratorDecay0::RMGGeneratorDecay0(RMGVVertexGenerator* prim_gen) : RMGVGenerator("Decay0") {

  if (!RMGManager::Instance()->IsExecSequential())
    RMGLog::Out(RMGLog::fatal, "BxDecay0 is not thread-safe (yet). Re-run in sequential mode.");

  if (!prim_gen) RMGLog::OutDev(RMGLog::fatal, "Primary position generator is nullptr");

  fDecay0G4Generator = std::make_unique<bxdecay0_g4::PrimaryGeneratorAction>();
  // NOTE: BxDecay0's primary generator action will own the pointer
  fDecay0G4Generator->SetVertexGenerator(prim_gen);
}

// Need non-inline, i.e. not in header/class body, destructor to hide BXDecay0 from consumers
RMGGeneratorDecay0::~RMGGeneratorDecay0() = default; // NOLINT

void RMGGeneratorDecay0::GeneratePrimariesKinematics(G4Event* event) {
  fDecay0G4Generator->GeneratePrimaries(event);
}

// vim: tabstop=2 shiftwidth=2 expandtab

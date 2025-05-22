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

#include <limits>

#include "Randomize.hh"

#include "RMGConfig.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGTools.hh"

#if RMG_HAS_BXDECAY0
#include "bxdecay0_g4/primary_generator_action.hh"
#endif

RMGGeneratorDecay0::RMGGeneratorDecay0(RMGVVertexGenerator* prim_gen) : RMGVGenerator("Decay0") {

#if !RMG_HAS_BXDECAY0_THREADSAFE
  if (!RMGManager::Instance()->IsExecSequential())
    RMGLog::Out(RMGLog::fatal, "BxDecay0 is not thread-safe (yet). Re-run in sequential mode.");
#endif

  if (!prim_gen) RMGLog::OutDev(RMGLog::fatal, "Primary position generator is nullptr");

  fDecay0G4Generator = std::make_unique<bxdecay0_g4::PrimaryGeneratorAction>();
  // NOTE: BxDecay0's primary generator action will own the pointer
  fDecay0G4Generator->SetVertexGenerator(prim_gen);
  this->DefineCommands();
}

// Need non-inline, i.e. not in header/class body, destructor to hide BXDecay0 from consumers
RMGGeneratorDecay0::~RMGGeneratorDecay0() = default; // NOLINT

void RMGGeneratorDecay0::GeneratePrimaries(G4Event* event) {
  fDecay0G4Generator->GeneratePrimaries(event);
}

void RMGGeneratorDecay0::SetMode(std::string mode) {
  try {
    fDecayMode = RMGTools::ToEnum<RMGGeneratorDecay0::DecayMode>(mode, "decay mode");
  } catch (const std::bad_cast&) { return; }
  if (fDecayMode == DecayMode::k2vbb) {
    nuclide = "Ge76";
    dbd_mode = 4;
    dbd_level = 0;
  } else if (fDecayMode == DecayMode::k0vbb) {
    nuclide = "Ge76";
    dbd_mode = 1;
    dbd_level = 0;
  } else {
    RMGLog::Out(RMGLog::error, "Unknown decay mode");
  }
  seed = static_cast<G4int>(G4UniformRand() * std::numeric_limits<G4int>::max());
  debug = false;

  bxdecay0_g4::PrimaryGeneratorAction::ConfigurationInterface& configInt = fDecay0G4Generator
                                                                               ->GrabConfiguration();

  configInt.reset_mdl();
  configInt.decay_category = "dbd";
  configInt.nuclide = nuclide;
  configInt.seed = seed;
  configInt.dbd_mode = dbd_mode;
  configInt.dbd_level = dbd_level;
  configInt.debug = debug;
  fDecay0G4Generator->SetConfigHasChanged(true);
}

void RMGGeneratorDecay0::DefineCommands() {

  // NOTE: SetUnit(Category) is not thread-safe

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Generator/Decay0/",
      "Commands for controlling the BxDecay0 generator"
  );

  fMessenger->DeclareMethod("Mode", &RMGGeneratorDecay0::SetMode)
      .SetGuidance("Set the mode of the BxDecay0 generator.")
      .SetParameterName("mode", false)
      .SetCandidates(RMGTools::GetCandidates<RMGGeneratorDecay0::DecayMode>())
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

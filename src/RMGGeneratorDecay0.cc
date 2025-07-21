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

#include "RMGGeneratorDecay0.hh"

#include <limits>

#include "Randomize.hh"

#include "RMGConfig.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGTools.hh"

#if RMG_HAS_BXDECAY0
#include "bxdecay0/bb_utils.h"
#include "bxdecay0_g4/primary_generator_action.hh"
#endif

RMGGeneratorDecay0::RMGGeneratorDecay0(RMGMasterGenerator* master_gen)
    : RMGVGenerator("Decay0"), fMasterGen(master_gen) {

#if !RMG_HAS_BXDECAY0_THREADSAFE
  if (!RMGManager::Instance()->IsExecSequential())
    RMGLog::Out(RMGLog::fatal, "BxDecay0 is not thread-safe (yet). Re-run in sequential mode.");
#endif

  if (!master_gen) RMGLog::OutDev(RMGLog::fatal, "Master generator is nullptr");

  fDecay0G4Generator = std::make_unique<bxdecay0_g4::PrimaryGeneratorAction>();
  this->DefineCommands();
}

// Need non-inline, i.e. not in header/class body, destructor to hide BXDecay0 from consumers
RMGGeneratorDecay0::~RMGGeneratorDecay0() = default; // NOLINT

void RMGGeneratorDecay0::GeneratePrimaries(G4Event* event) {
  fDecay0G4Generator->GeneratePrimaries(event);
}

void RMGGeneratorDecay0::BeginOfRunAction(const G4Run*) {

  RMGVVertexGenerator* prim_gen = fMasterGen->GetVertexGenerator();
  if (!prim_gen) {
    RMGLog::Out(RMGLog::fatal, "No primary vertex generator specified at runtime for BxDecay0!");
  }
  // Set the vertex generator by reference to not lose ownership.
  fDecay0G4Generator->SetVertexGenerator(*prim_gen);

  // Technically the user can change the seed at any time, so we have to always update.
  // But only actually update the seed once the user used a RMGGeneratorDecay0 command.
  if (fUpdateSeeds) {
    long seed = CLHEP::HepRandom::getTheSeed();
    // Sanity check
    if (seed <= 0) {
      RMGLog::Out(
          RMGLog::error,
          "CLHEP seed seems invalid or uninitialized. Setting BxDecay0 seed to 0."
      );
      seed = 0;
    }

    // This should never occur on most systems, as the only way is through the internal CLHEP table
    // and the values there are all below the max int32.
    if (seed > std::numeric_limits<int>::max()) {
      long new_seed = seed - std::numeric_limits<int>::max();
      RMGLog::Out(
          RMGLog::warning,
          "Seed ",
          seed,
          " is too large for BxDecay0. Largest possible seed is ",
          std::numeric_limits<int>::max(),
          ". Setting BxDecay0 seed to ",
          new_seed
      );
      seed = new_seed;
    }

    bxdecay0_g4::PrimaryGeneratorAction::ConfigurationInterface&
        configInt = fDecay0G4Generator->GrabConfiguration();
    configInt.seed = static_cast<int>(seed);
    RMGLog::Out(RMGLog::debug, "BxDecay0 generator: seed set to ", seed);
    fDecay0G4Generator->SetConfigHasChanged(true);
  }
}

void RMGGeneratorDecay0::EndOfRunAction(const G4Run*) {
  // Reset the vertex generator to avoid using stale objects
  fDecay0G4Generator->SetVertexGenerator(nullptr);
}

void RMGGeneratorDecay0::SetBackground(std::string isotope) {
  RMGLog::Out(RMGLog::debug, "Setting BxDecay0 background mode with isotope: ", isotope);
  bxdecay0_g4::PrimaryGeneratorAction::ConfigurationInterface& configInt = fDecay0G4Generator
                                                                               ->GrabConfiguration();
  configInt.reset_base();
  configInt.decay_category = "background";
  configInt.nuclide = isotope;
  configInt.debug = false; // If we want a debug mode, we need to add a command for it
  fUpdateSeeds = true;     // The seed and config update should now be done in BeginOfRunAction
}

void RMGGeneratorDecay0::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Generator/BxDecay0/",
      "Commands for controlling the BxDecay0 generator"
  );
  std::string isot_cand;
  for (const auto& isot : bxdecay0::background_isotopes()) { isot_cand += std::string(isot) + ' '; }
  // Need an additional command because both modes expect different parameters.
  fMessenger->DeclareMethod("Background", &RMGGeneratorDecay0::SetBackground)
      .SetGuidance("Set the isotope for the Background mode of the BxDecay0 generator. E.g. 'Co60'")
      .SetParameterName("isotope", false)
      .SetCandidates(isot_cand)
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);

  // Make one macro command for the double beta decay mode.
  fUIMessenger = std::make_unique<BxMessenger>(this);
}

RMGGeneratorDecay0::BxMessenger::BxMessenger(RMGGeneratorDecay0* gen) : fGen(gen) {


  fGeneratorCmd = new G4UIcommand("/RMG/Generator/BxDecay0/DoubleBetaDecay", this);
  fGeneratorCmd
      ->SetGuidance("Set the isotope, process and energy level for the double beta decay mode of the BxDecay0 generator");

  auto isotope = new G4UIparameter("isotope", 's', false);
  isotope->SetGuidance("Set the isotope for the double beta decay");
  std::string isot_cand;
  for (const auto& isot : bxdecay0::dbd_isotopes()) { isot_cand += std::string(isot) + ' '; }
  isotope->SetParameterCandidates(isot_cand.c_str());
  fGeneratorCmd->SetParameter(isotope);

  auto process = new G4UIparameter("process", 's', false);
  process->SetGuidance("Name the decay process you want to simulate");
  process->SetParameterCandidates(RMGTools::GetCandidates<RMGGeneratorDecay0::Process>().c_str());
  fGeneratorCmd->SetParameter(process);

  auto level = new G4UIparameter("level", 'i', false);
  level->SetGuidance("Energy level of the daughter nucleus");
  level->SetDefaultValue("0");
  level->SetOmittable(true);
  fGeneratorCmd->SetParameter(level);

  fGeneratorCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

RMGGeneratorDecay0::BxMessenger::~BxMessenger() { delete fGeneratorCmd; }

void RMGGeneratorDecay0::BxMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
  if (command == fGeneratorCmd) GeneratorCmd(newValues); // This way it is scalable to more commands
}

void RMGGeneratorDecay0::BxMessenger::GeneratorCmd(const std::string& parameters) {
  G4Tokenizer next(parameters);

  auto isotope = next();
  auto proc = next();
  auto level = std::stoi(next());
  RMGLog::Out(RMGLog::debug, "BxDecay0 generator: isotope = ", isotope, ", proc = ", proc, ", level = ", level);
  Process p = RMGGeneratorDecay0::Process::k2vbb;
  try {
    p = RMGTools::ToEnum<RMGGeneratorDecay0::Process>(proc, "process");
  } catch (const std::bad_cast&) { return; }

  bxdecay0_g4::PrimaryGeneratorAction::ConfigurationInterface& configInt = fGen->fDecay0G4Generator
                                                                               ->GrabConfiguration();
  configInt.reset_mdl();
  configInt.decay_category = "dbd";
  configInt.nuclide = isotope;
  configInt.dbd_mode = static_cast<int>(p);
  configInt.dbd_level = level;
  configInt.debug = false;   // If we want a debug mode, we need to add it to the command
  fGen->fUpdateSeeds = true; // The seed and config update should now be done in BeginOfRunAction
}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "RMGGeneratorPrimaryMessenger.hh"

#include "globals.hh"
#include "G4PhysicalVolumeStore.hh"

#include "RMGVGenerator.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGGeneratorG4Gun.hh"
#include "RMGGeneratorSPS.hh"
#include "RMGTools.hh"
#include "RMGLog.hh"
#include "ProjectInfo.hh"
#if RMG_HAS_BXDECAY0
#include "RMGGeneratorDecay0.hh"
#endif

RMGGeneratorPrimaryMessenger::RMGGeneratorPrimaryMessenger(RMGGeneratorPrimary* generator) :
  fGeneratorPrimary(generator) {

  G4String directory = "/RMG/Generator";
  fGeneratorDirectory = std::unique_ptr<G4UIdirectory>(new G4UIdirectory(directory));

  G4String generators = "SPS G4Gun";
#if RMG_HAS_BXDECAY0
  generators += " Decay0";
#endif

  fSelectCmd = RMGTools::MakeG4UIcmdWithAString(
      directory + "/Select", this, generators, {G4State_Init, G4State_PreInit});

  fConfineCmd = RMGTools::MakeG4UIcmdWithAString(directory + "/Confine", this,
    "UnConfined Volume");
}

void RMGGeneratorPrimaryMessenger::SetNewValue(G4UIcommand* cmd, G4String new_values) {

  if (cmd == fSelectCmd.get()) {
    if (new_values == "G4gun") {
      fGeneratorPrimary->SetGenerator(new RMGGeneratorG4Gun);
    }
    else if (new_values == "SPS") {
      fGeneratorPrimary->SetGenerator(new RMGGeneratorSPS);
    }
#if RMG_HAS_BXDECAY0
    else if (new_values == "Decay0") {
      fGeneratorPrimary->SetGenerator(new RMGGeneratorDecay0);
    }
#endif
    else RMGLog::Out(RMGLog::fatal, "Unknown generator '", new_values, "'");
  }
  else if (cmd == fConfineCmd.get()) {
    if (new_values == "Volume") fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::ConfinementCode::kVolume);
    if (new_values == "UnConfined") fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::ConfinementCode::kUnConfined);
  }
  // else if
}

// vim: tabstop=2 shiftwidth=2 expandtab

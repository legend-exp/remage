#include "RMGGeneratorPrimaryMessenger.hh"

#include "globals.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4PhysicalVolumeStore.hh"

#include "RMGVGenerator.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGGeneratorG4Gun.hh"
#include "RMGGeneratorSPS.hh"
#include "RMGTools.hh"
#include "RMGLog.hh"

RMGGeneratorPrimaryMessenger::RMGGeneratorPrimaryMessenger(RMGGeneratorPrimary* generator) :
  fGeneratorPrimary(generator) {

  G4String directory = "/RMG/Generator";
  fGeneratorDirectory = std::unique_ptr<G4UIdirectory>(new G4UIdirectory(directory));

  fSelectCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(
      directory + "/Select", this, "SPS G4Gun");

  fConfineCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(directory + "/Confine", this,
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
    // else if...
    else RMGLog::Out(RMGLog::fatal, "Unknown generator '", new_values, "'");
  }
  else if (cmd == fConfineCmd.get()) {
    if (new_values == "Volume") fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::ConfinementCode::kVolume);
    if (new_values == "UnConfined") fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::ConfinementCode::kUnConfined);
  }
  // else if
}

// vim: tabstop=2 shiftwidth=2 expandtab
#include "RMGManagerMessenger.hh"

#include <random>

#include "globals.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithABool.hh"
#include "Randomize.hh"

#include "RMGManager.hh"
#include "RMGManagementRunAction.hh"
#include "RMGLog.hh"
#include "RMGTools.hh"

RMGManagerMessenger::RMGManagerMessenger(RMGManager*) {

  G4String directory = "/RMG/Manager";
  fDirectories.emplace_back(new G4UIdirectory(directory));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Logging").c_str()));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Randomization").c_str()));

  fScreenLogCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(directory + "/Logging/LogLevelScreen", this,
      "Debug Detail Summary Warning Error Fatal");

  fFileNameLogCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(directory + "/Logging/LogToFile", this,
      "");

  fFileLogCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(directory + "/Logging/LogLevelFile", this,
      "Debug Detail Summary Warning Error Fatal");

  fHEPRandomSeedCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAnInteger>(directory + "/Randomization/Seed", this,
      "seed", "seed >= 0");

  fUseInternalSeedCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAnInteger>(
      directory + "/UseInternalSeed", this, "index", "index >= 0 && index < 430");

  fSeedWithDevRandomCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithABool>(
      directory + "/Randomization/UseSystemEntropySeed", this, true, true);

  fUseRandomEngineCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(directory + "/Randomization/RandomEngine",
      this, "JamesRandom RanLux MTwist");
}

void RMGManagerMessenger::SetNewValue(G4UIcommand* cmd, G4String new_values) {

  if (cmd == fScreenLogCmd.get()) {
    if      (new_values == "Debug"  ) RMGLog::SetLogLevelScreen(RMGLog::debug);
    else if (new_values == "Detail" ) RMGLog::SetLogLevelScreen(RMGLog::detail);
    else if (new_values == "Summary") RMGLog::SetLogLevelScreen(RMGLog::summary);
    else if (new_values == "Warning") RMGLog::SetLogLevelScreen(RMGLog::warning);
    else if (new_values == "Error"  ) RMGLog::SetLogLevelScreen(RMGLog::error);
    else if (new_values == "Fatal"  ) RMGLog::SetLogLevelScreen(RMGLog::fatal);
    else RMGLog::Out(RMGLog::error, "Unknown logging level '", new_values, "'");
  }
  else if (cmd == fFileNameLogCmd.get()) {
    RMGLog::OpenLogFile(new_values);
  }
  else if (cmd == fFileLogCmd.get()) {
    if      (new_values == "Debug"  ) RMGLog::SetLogLevelFile(RMGLog::debug);
    else if (new_values == "Detail" ) RMGLog::SetLogLevelFile(RMGLog::detail);
    else if (new_values == "Summary") RMGLog::SetLogLevelFile(RMGLog::summary);
    else if (new_values == "Warning") RMGLog::SetLogLevelFile(RMGLog::warning);
    else if (new_values == "Error"  ) RMGLog::SetLogLevelFile(RMGLog::error);
    else if (new_values == "Fatal"  ) RMGLog::SetLogLevelFile(RMGLog::fatal);
    else RMGLog::Out(RMGLog::error, "Unknown logging level '", new_values, "'");
  }
  else if (cmd == fHEPRandomSeedCmd.get()) {

    G4long seed = fHEPRandomSeedCmd->GetNewIntValue(new_values);

    if (seed >= std::numeric_limits<long>::max()) {
      RMGLog::Out(RMGLog::error, "Seed ", new_values, " is too large. Largest possible seed is ",
          std::numeric_limits<long>::max(), ". Setting seed to 0.");
      CLHEP::HepRandom::setTheSeed(0);
    }
    else CLHEP::HepRandom::setTheSeed(seed);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", seed);
    RMGManager::GetRMGManager()->GetRMGRunAction()->SetControlledRandomization();
  }
  else if (cmd == fUseInternalSeedCmd.get()) {

    auto index = fUseInternalSeedCmd->GetNewIntValue(new_values);

    long seeds[2];
    int table_index = index/2;
    CLHEP::HepRandom::getTheTableSeeds(seeds, table_index);

    int array_index = index % 2;
    CLHEP::HepRandom::setTheSeed(seeds[array_index]);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", seeds[array_index]);
    RMGManager::GetRMGManager()->GetRMGRunAction()->SetControlledRandomization();
  }
  else if (cmd == fSeedWithDevRandomCmd.get()) {
    std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
    std::random_device rd; // uses RDRND or /dev/urandom
    auto rand_seed = dist(rd);
    CLHEP::HepRandom::setTheSeed(rand_seed);
    RMGLog::Out(RMGLog::summary, "CLHEP::HepRandom seed set to: ", rand_seed);
    RMGManager::GetRMGManager()->GetRMGRunAction()->SetControlledRandomization();
  }
  else if (cmd == fUseRandomEngineCmd.get()) {

    if (new_values == "JamesRandom") {
      CLHEP::HepRandom::setTheEngine(new CLHEP::HepJamesRandom);
      RMGLog::Out(RMGLog::summary, "Using James random engine");
    }
    else if (new_values == "RanLux") {
      CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine);
      RMGLog::Out(RMGLog::summary, "Using RanLux random engine");
    }
    else if (new_values == "MTwist") {
      CLHEP::HepRandom::setTheEngine(new CLHEP::MTwistEngine);
      RMGLog::Out(RMGLog::summary, "Using MTwist random engine");
    }
    else {
      CLHEP::HepRandom::setTheEngine(new CLHEP::HepJamesRandom);
      RMGLog::Out(RMGLog::error, new_values, " random engine unknown, using defaults");
      RMGLog::Out(RMGLog::summary, "Using James random engine");
    }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "RMGProcessesMessenger.hh"

#include <map>
#include <exception>

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "globals.hh"
#include "G4ProcessManager.hh"
#include "G4StepLimiter.hh"

#include "RMGProcessesList.hh"
#include "RMGUIcmdStepLimit.hh"
#include "RMGManager.hh"
#include "RMGManagerDetectorConstruction.hh"
#include "RMGTools.hh"
#include "RMGLog.hh"

RMGProcessesMessenger::RMGProcessesMessenger(RMGProcessesList *plist) :
  fProcessesList(plist) {

  G4String directory = "/RMG/Processes";

  fProcessesDir = std::unique_ptr<G4UIdirectory>(new G4UIdirectory(directory));

  fRealmCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(directory + "/Realm", this,
      "BBdecay DarkMatter CosmicRays OpticalPhoton");

  fOpticalProcessesCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithABool>(directory + "/OpticalPhysics", this);

  fOpticalOnlyCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithABool>(directory + "/OpticalPhysicsOnly", this);

  fLowEnergyProcessesCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithABool>(directory + "/LowEnergyEMPhysics", this);

  fLowEnergyProcessesOptionCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>("/LowEnergyEMPhysicsOption", this,
      "Livermore EmOption1 EmOption2 EmOption3 EmOption4 Penelope LivermorePolarized");

  fStepLimitCmd = std::unique_ptr<RMGUIcmdStepLimit>(new RMGUIcmdStepLimit(directory + "/SetStepLimit", this));

  fUseAngCorrCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithABool>(directory + "/AngularCorrelation", this);

  fSetAngCorrCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAnInteger>(directory + "/TwoJMAX", this,
      "x", "x >= 0");

  fStoreICLevelData = RMGTools::MakeG4UIcmd<G4UIcmdWithABool>(directory + "/StoreICLevelData", this);
}

void RMGProcessesMessenger::SetNewValue(G4UIcommand *cmd, G4String new_val) {

  if (cmd == fRealmCmd.get()) {
    fProcessesList->SetRealm(new_val);
  }
  else if (cmd == fOpticalProcessesCmd.get()) {
    fProcessesList->SetOpticalFlag(fOpticalProcessesCmd->GetNewBoolValue(new_val));
  }
  else if (cmd == fOpticalOnlyCmd.get()) {
    fProcessesList->SetOpticalPhysicsOnly(fOpticalOnlyCmd->GetNewBoolValue(new_val));
  }
  else if (cmd == fLowEnergyProcessesCmd.get()) {
    fProcessesList->SetLowEnergyFlag(fLowEnergyProcessesCmd->GetNewBoolValue(new_val));
  }
  else if (cmd == fLowEnergyProcessesOptionCmd.get()) {
    std::map<G4String, G4int> options = {
      { "Livermore",          0 },
      { "EmOption1",          1 },
      { "EmOption2",          2 },
      { "EmOption3",          3 },
      { "EmOption4",          4 },
      { "Penelope",           5 },
      { "LivermorePolarized", 6 }
    };

    try {
      fProcessesList->SetLowEnergyOption(options.at(new_val));
    }
    catch (const std::exception& e) {
      RMGLog::Out(RMGLog::debug, e);
      RMGLog::Out(RMGLog::fatal, "'", new_val, "' low energy option not known");
    }
  }
  else if (cmd == fStepLimitCmd.get()) {
    G4String particle_name = fStepLimitCmd->GetParticleName(new_val);
    fProcessesList->LimitStepForParticle(particle_name);

    G4String volume_name = fStepLimitCmd->GetVolumeName(new_val);
    G4double max_step = fStepLimitCmd->GetStepSize(new_val);
    RMGManager::GetRMGManager()->GetManagerDetectorConstruction()->SetMaxStepLimit(volume_name, max_step);
  }
  else if (cmd == fUseAngCorrCmd.get()) {
    fProcessesList->SetUseAngCorr(fUseAngCorrCmd->GetNewBoolValue(new_val));
  }
  else if (cmd == fStoreICLevelData.get()) {
    fProcessesList->SetStoreICLevelData(fStoreICLevelData->GetNewBoolValue(new_val));
  }
}

// vim: shiftwidth=2 tabstop=2 expandtab

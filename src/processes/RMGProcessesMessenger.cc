#include "RMGProcessesMessenger.hh"

#include <map>
#include <exception>

#include "globals.hh"
#include "G4ProcessManager.hh"
#include "G4StepLimiter.hh"

#include "RMGProcessesList.hh"
#include "RMGManager.hh"
#include "RMGManagementDetectorConstruction.hh"
#include "RMGTools.hh"
#include "RMGLog.hh"

RMGProcessesMessenger::RMGProcessesMessenger(RMGProcessesList* plist) :
  fProcessesList(plist) {

  G4String directory = "/RMG/Processes/";

  fProcessesDir = std::make_unique<G4UIdirectory>(directory);

  fStepLimitCmd = std::make_unique<RMGUIcmdStepLimit>(directory + "SetStepLimit", this);
}

void RMGProcessesMessenger::SetNewValue(G4UIcommand *cmd, G4String new_val) {

  if (cmd == fStepLimitCmd.get()) {
    G4String particle_name = fStepLimitCmd->GetParticleName(new_val);
    fProcessesList->LimitStepForParticle(particle_name);

    G4String volume_name = fStepLimitCmd->GetVolumeName(new_val);
    G4double max_step = fStepLimitCmd->GetStepSize(new_val);
    RMGManager::GetRMGManager()->GetManagementDetectorConstruction()->SetMaxStepLimit(volume_name, max_step);
  }
}

// vim: shiftwidth=2 tabstop=2 expandtab

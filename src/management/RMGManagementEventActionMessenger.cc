#include "RMGManagementEventActionMessenger.hh"

#include "G4UIcommand.hh"
#include "G4UImessenger.hh"

#include "RMGManagementEventAction.hh"
#include "RMGVOutputManager.hh"
#include "RMGLog.hh"
#include "RMGTools.hh"

RMGManagementEventActionMessenger::RMGManagementEventActionMessenger(RMGManagementEventAction *eventaction) :
  fEventAction(eventaction) {

  G4String directory = "/RMG/Output";
  fEventDirectory = std::unique_ptr<G4UIdirectory>(new G4UIdirectory(directory));

  fSetFileNameCmd = RMGTools::MakeG4UIcmdWithAString(directory + "/FileName", this);
}

void RMGManagementEventActionMessenger::SetNewValue(G4UIcommand* cmd, G4String new_values) {

  if (cmd == fSetFileNameCmd.get()) {
     if (fEventAction->GetOutputManager()) {
        fEventAction->GetOutputManager()->SetFileName(new_values);
     }
     else {
       RMGLog::Out(RMGLog::fatal, "No output scheme defined!");
     }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

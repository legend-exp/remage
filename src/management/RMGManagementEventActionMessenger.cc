#include "RMGManagementEventActionMessenger.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UImessenger.hh"

#include "RMGManagementEventAction.hh"
#include "RMGVOutputManager.hh"
#include "RMGLog.hh"
#include "RMGTools.hh"

RMGManagementEventActionMessenger::RMGManagementEventActionMessenger(RMGManagementEventAction *eventaction) :
  fEventAction(eventaction) {

  G4String directory = "/RMG/Output";
  fEventDirectory = std::unique_ptr<G4UIdirectory>(new G4UIdirectory(directory));

  fSetFileNameCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(directory + "/FileName", this);

  fSetReportingFrequencyCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAnInteger>(directory + "/ReportingFrequency", this,
      "x", "x > 0");

  fSetWriteOutFrequencyCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAnInteger>(directory + "/WriteOutFrequency", this,
      "x", "x > 0");

  fSetWriteOutFileDuringRunCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithABool>(directory + "/WriteOutFileDuringRun", this);
}

void RMGManagementEventActionMessenger::SetNewValue(G4UIcommand* cmd, G4String new_values) {

  if (cmd == fSetReportingFrequencyCmd.get()) {
    fEventAction->SetReportingFrequency(fSetReportingFrequencyCmd->GetNewIntValue(new_values));
  }
  else if (cmd == fSetWriteOutFrequencyCmd.get()) {
    fEventAction->SetWriteOutFrequency(fSetWriteOutFrequencyCmd->GetNewIntValue(new_values));
  }
  else if (cmd == fSetWriteOutFileDuringRunCmd.get()) {
    fEventAction->SetWriteOutFileDuringRun(fSetWriteOutFileDuringRunCmd->GetNewBoolValue(new_values));
  }
  else if (cmd == fSetFileNameCmd.get()) {
     if (fEventAction->GetOutputManager()) {
        fEventAction->GetOutputManager()->SetFileName(new_values);
     }
     else {
       RMGLog::Out(RMGLog::fatal, "No output scheme defined!");
     }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

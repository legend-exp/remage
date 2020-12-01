#include "RMGManagementEventActionMessenger.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UImessenger.hh"

#include "RMGManagementEventAction.hh"
#include "RMGVOutputManager.hh"
#include "RMGLog.hh"

RMGManagementEventActionMessenger::RMGManagementEventActionMessenger(RMGManagementEventAction *eventaction) :
  fEventAction(eventaction) {

  fEventDirectory = new G4UIdirectory("/RMG/eventaction/");
  fEventDirectory->SetGuidance("Parameters for the RMGManagementEventAction class");
  fEventDirectory->SetGuidance("Controls what happens before, during and after each event.");
  fEventDirectory->SetGuidance("Determines output format (Root, etc.) and schema.");

  fSetFileNameCmd = new G4UIcmdWithAString("/RMG/eventaction/rootfilename", this);
  fSetFileNameCmd->SetGuidance("Name for output file.");

  fSetReportingFrequencyCmd = new G4UIcmdWithAnInteger("/RMG/eventaction/reportingfrequency", this);
  fSetReportingFrequencyCmd->SetGuidance("Set number of events between reporting current event #");

  fSetWriteOutFrequencyCmd = new G4UIcmdWithAnInteger("/RMG/eventaction/writeOutFrequency", this);
  fSetWriteOutFrequencyCmd->SetGuidance("Set number of events between writing out data file.  Will default to reporting frequency.");

  fSetWriteOutFileDuringRunCmd = new G4UIcmdWithABool("/RMG/eventaction/writeOutFileDuringRun", this);
  fSetWriteOutFileDuringRunCmd->SetGuidance("Sets data file writing during run.");
  fSetWriteOutFileDuringRunCmd->SetDefaultValue(true);
}

RMGManagementEventActionMessenger::~RMGManagementEventActionMessenger() {
  delete fEventDirectory;
  delete fGetOutputSchemaCmd;
  delete fSetFileNameCmd;
  delete fSetSchemaCmd;
  delete fSetReportingFrequencyCmd;
  delete fSetWriteOutFrequencyCmd;
  delete fSetWriteOutFileDuringRunCmd;
}

void RMGManagementEventActionMessenger::SetNewValue(G4UIcommand *command, G4String new_values) {

  if (command == fSetReportingFrequencyCmd) {
    fEventAction->SetReportingFrequency(fSetReportingFrequencyCmd->GetNewIntValue(new_values));
  }
  else if (command == fSetWriteOutFrequencyCmd) {
    fEventAction->SetWriteOutFrequency(fSetWriteOutFrequencyCmd->GetNewIntValue(new_values));
  }
  else if (command == fSetWriteOutFileDuringRunCmd) {
    fEventAction->SetWriteOutFileDuringRun(fSetWriteOutFileDuringRunCmd->GetNewBoolValue(new_values));
  }
  else if (command == fSetFileNameCmd) {
     if (fEventAction->GetOutputManager()) {
        fEventAction->GetOutputManager()->SetFileName(new_values);
     }
     else {
        // RMGLog(error) << "Neither pre-waveform nor normal output class defined."
        //              << "If you want to create a normal output file, please define a schema via"
        //              << "    /RMG/eventaction/rootschema"
        //              << "If you want to create a pre-waveform output file, please choose a format via"
        //              << "    /RMG/eventaction/PreWaveformFormat" << endlog;
     }
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

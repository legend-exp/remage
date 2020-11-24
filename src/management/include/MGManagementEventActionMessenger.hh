#ifndef _MGMANAGEMENTEVENTACTIONMESSENGER_HH_
#define _MGMANAGEMENTEVENTACTIONMESSENGER_HH_

#include "G4UImessenger.hh"

class MGManagementEventAction;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithABool;
class MGManagementEventActionMessenger: public G4UImessenger  {

  public:

    MGManagementEventActionMessenger(MGManagementEventAction *eventaction);
    ~MGManagementEventActionMessenger();

    MGManagementEventActionMessenger           (MGManagementEventActionMessenger const&) = delete;
    MGManagementEventActionMessenger& operator=(MGManagementEventActionMessenger const&) = delete;
    MGManagementEventActionMessenger           (MGManagementEventActionMessenger&&)      = delete;
    MGManagementEventActionMessenger& operator=(MGManagementEventActionMessenger&&)      = delete;

    void SetNewValue(G4UIcommand *command, G4String newValues) override;

  private:

    MGManagementEventAction* fEventAction;

    G4UIdirectory*           fEventDirectory;
    G4UIcmdWithoutParameter* fGetOutputSchemaCmd;
    G4UIcmdWithAString*      fSetFileNameCmd;
    G4UIcmdWithAString*      fSetSchemaCmd;
    G4UIcmdWithAnInteger*    fSetReportingFrequencyCmd;
    G4UIcmdWithAnInteger*    fSetWriteOutFrequencyCmd;
    G4UIcmdWithABool*        fSetWriteOutFileDuringRunCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

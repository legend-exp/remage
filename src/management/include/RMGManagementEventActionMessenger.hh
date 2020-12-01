#ifndef _RMGMANAGEMENTEVENTACTIONMESSENGER_HH_
#define _RMGMANAGEMENTEVENTACTIONMESSENGER_HH_

#include "G4UImessenger.hh"

class RMGManagementEventAction;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithABool;
class RMGManagementEventActionMessenger: public G4UImessenger  {

  public:

    RMGManagementEventActionMessenger(RMGManagementEventAction *eventaction);
    ~RMGManagementEventActionMessenger();

    RMGManagementEventActionMessenger           (RMGManagementEventActionMessenger const&) = delete;
    RMGManagementEventActionMessenger& operator=(RMGManagementEventActionMessenger const&) = delete;
    RMGManagementEventActionMessenger           (RMGManagementEventActionMessenger&&)      = delete;
    RMGManagementEventActionMessenger& operator=(RMGManagementEventActionMessenger&&)      = delete;

    void SetNewValue(G4UIcommand *command, G4String newValues) override;

  private:

    RMGManagementEventAction* fEventAction;

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

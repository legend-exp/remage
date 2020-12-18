#ifndef _RMG_MANAGEMENT_EVENT_ACTION_MESSENGER_HH_
#define _RMG_MANAGEMENT_EVENT_ACTION_MESSENGER_HH_

#include <memory>

#include "G4UImessenger.hh"

class RMGManagementEventAction;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcommand;
class G4UIdirectory;
class RMGManagementEventActionMessenger: public G4UImessenger  {

  public:

    RMGManagementEventActionMessenger(RMGManagementEventAction*);
    ~RMGManagementEventActionMessenger();

    RMGManagementEventActionMessenger           (RMGManagementEventActionMessenger const&) = delete;
    RMGManagementEventActionMessenger& operator=(RMGManagementEventActionMessenger const&) = delete;
    RMGManagementEventActionMessenger           (RMGManagementEventActionMessenger&&)      = delete;
    RMGManagementEventActionMessenger& operator=(RMGManagementEventActionMessenger&&)      = delete;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:

    RMGManagementEventAction* fEventAction;

    std::unique_ptr<G4UIdirectory> fEventDirectory;

    std::unique_ptr<G4UIcmdWithAString>      fSetFileNameCmd;
    std::unique_ptr<G4UIcmdWithAString>      fSetSchemaCmd;
    std::unique_ptr<G4UIcmdWithAnInteger>    fSetReportingFrequencyCmd;
    std::unique_ptr<G4UIcmdWithAnInteger>    fSetWriteOutFrequencyCmd;
    std::unique_ptr<G4UIcmdWithABool>        fSetWriteOutFileDuringRunCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMG_MANAGEMENT_EVENT_ACTION_MESSENGER_HH_
#define _RMG_MANAGEMENT_EVENT_ACTION_MESSENGER_HH_

#include <memory>

#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"

class RMGManagementEventAction;
class G4UIcommand;
class RMGManagementEventActionMessenger: public G4UImessenger  {

  public:

    RMGManagementEventActionMessenger(RMGManagementEventAction*);
    ~RMGManagementEventActionMessenger() = default;

    RMGManagementEventActionMessenger           (RMGManagementEventActionMessenger const&) = delete;
    RMGManagementEventActionMessenger& operator=(RMGManagementEventActionMessenger const&) = delete;
    RMGManagementEventActionMessenger           (RMGManagementEventActionMessenger&&)      = delete;
    RMGManagementEventActionMessenger& operator=(RMGManagementEventActionMessenger&&)      = delete;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:

    RMGManagementEventAction* fEventAction;

    std::unique_ptr<G4UIdirectory> fEventDirectory;

    std::unique_ptr<G4UIcmdWithAString> fSetFileNameCmd;
    std::unique_ptr<G4UIcmdWithAString> fSetSchemaCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

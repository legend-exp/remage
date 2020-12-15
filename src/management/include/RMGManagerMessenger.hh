#ifndef _RMG_MANAGER_MESSENGER_HH_
#define _RMG_MANAGER_MESSENGER_HH_

#include <vector>
#include <memory>

#include "globals.hh"
#include "G4UImessenger.hh"

class RMGManager;
class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool;
class G4UIdirectory;
class RMGManagerMessenger: public G4UImessenger {

  public:

    RMGManagerMessenger(RMGManager *manager);
    ~RMGManagerMessenger();

    RMGManagerMessenger           (RMGManagerMessenger const&) = delete;
    RMGManagerMessenger& operator=(RMGManagerMessenger const&) = delete;
    RMGManagerMessenger           (RMGManagerMessenger&&)      = delete;
    RMGManagerMessenger& operator=(RMGManagerMessenger&&)      = delete;

    void SetNewValue(G4UIcommand* command, G4String new_values) override;

  private:

    std::vector<std::unique_ptr<G4UIdirectory>> fDirectories;

    std::unique_ptr<G4UIcmdWithAString>   fScreenLogCmd;
    std::unique_ptr<G4UIcmdWithAString>   fFileNameLogCmd;
    std::unique_ptr<G4UIcmdWithAString>   fFileLogCmd;
    std::unique_ptr<G4UIcmdWithAString>   fUseRandomEngineCmd;
    std::unique_ptr<G4UIcmdWithAnInteger> fHEPRandomSeedCmd;
    std::unique_ptr<G4UIcmdWithAnInteger> fUseInternalSeedCmd;
    std::unique_ptr<G4UIcmdWithABool>     fSeedWithDevRandomCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

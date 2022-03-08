#ifndef _RMG_PROCESSES_MESSENGER_HH_
#define _RMG_PROCESSES_MESSENGER_HH_

#include <memory>

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "RMGUIcmdStepLimit.hh"

class RMGProcessesList;
class RMGProcessesMessenger : public G4UImessenger {

  public:

    RMGProcessesMessenger(RMGProcessesList*);
    ~RMGProcessesMessenger() = default;

    RMGProcessesMessenger           (RMGProcessesMessenger const&) = delete;
    RMGProcessesMessenger& operator=(RMGProcessesMessenger const&) = delete;
    RMGProcessesMessenger           (RMGProcessesMessenger&&)      = delete;
    RMGProcessesMessenger& operator=(RMGProcessesMessenger&&)      = delete;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:

    RMGProcessesList* fProcessesList;
    std::unique_ptr<G4UIdirectory> fProcessesDir;
    std::unique_ptr<RMGUIcmdStepLimit> fStepLimitCmd;
};

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

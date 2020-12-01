#ifndef _RMGMANAGERMESSENGER_HH
#define _RMGMANAGERMESSENGER_HH

#include "globals.hh"
#include "G4UIcommand.hh"
#include "G4UImessenger.hh"
#include "RMGManager.hh"

class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithoutParameter;
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

    G4UIdirectory*           fDirectory;
    G4UIcmdWithAString*      fRMGLogCmd;
    G4UIcmdWithAString*      fUseRandomEngineCmd;
    G4UIcmdWithAnInteger*    fHEPRandomSeedCmd;
    G4UIcmdWithAnInteger*    fUseInternalSeedCmd;
    G4UIcmdWithoutParameter* fSeedWithDevRandomCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

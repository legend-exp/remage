#ifndef _MGMANAGERMESSENGER_HH
#define _MGMANAGERMESSENGER_HH

#include "globals.hh"
#include "G4UIcommand.hh"
#include "G4UImessenger.hh"
#include "MGManager.hh"

class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithoutParameter;
class G4UIdirectory;
class MGManagerMessenger: public G4UImessenger {

  public:

    MGManagerMessenger(MGManager *manager);
    ~MGManagerMessenger();

    MGManagerMessenger           (MGManagerMessenger const&) = delete;
    MGManagerMessenger& operator=(MGManagerMessenger const&) = delete;
    MGManagerMessenger           (MGManagerMessenger&&)      = delete;
    MGManagerMessenger& operator=(MGManagerMessenger&&)      = delete;

    void SetNewValue(G4UIcommand* command, G4String new_values) override;

  private:

    G4UIdirectory*           fDirectory;
    G4UIcmdWithAString*      fMGLogCmd;
    G4UIcmdWithAString*      fUseRandomEngineCmd;
    G4UIcmdWithAnInteger*    fHEPRandomSeedCmd;
    G4UIcmdWithAnInteger*    fUseInternalSeedCmd;
    G4UIcmdWithoutParameter* fSeedWithDevRandomCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

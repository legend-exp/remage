#ifndef _MGPROCESSESMESSENGER_HH
#define _MGPROCESSESMESSENGER_HH

#include "globals.hh"
#include "G4UImessenger.hh"

class MGProcessesList;

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIcmdWithABool;
// class MGUIcmdStepLimit;
// class G4UIcmdWithoutParameter;
class G4UIcmdWithAnInteger;
class MGProcessesMessenger : public G4UImessenger {

  public:

    MGProcessesMessenger(MGProcessesList*);
    ~MGProcessesMessenger() override;

    MGProcessesMessenger           (MGProcessesMessenger const&) = delete;
    MGProcessesMessenger& operator=(MGProcessesMessenger const&) = delete;
    MGProcessesMessenger           (MGProcessesMessenger&&)      = delete;
    MGProcessesMessenger& operator=(MGProcessesMessenger&&)      = delete;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:

    MGProcessesList*         fProcessesList;
    G4UIdirectory*           fMGProcessesDir;
    G4UIcmdWithAString*      fMGProcessesChoiceCmd;
    G4UIcmdWithABool*        fOpticalProcessesCmd;
    G4UIcmdWithABool*        fOpticalOnlyCmd;
    G4UIcmdWithABool*        fLowEnergyProcessesCmd;
    G4UIcmdWithAnInteger*    fLowEnergyProcessesOptionCmd;
    // G4UIcmdWithoutParameter* fGetStepLimitCmd;
    // MGUIcmdStepLimit*        fStepLimitCmd;
    G4UIcmdWithAnInteger*    fAngCorrCmd;
    G4UIcmdWithABool*        fStoreICLevelData;
};

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

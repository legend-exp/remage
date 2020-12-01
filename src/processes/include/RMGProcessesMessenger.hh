#ifndef _RMGPROCESSESMESSENGER_HH
#define _RMGPROCESSESMESSENGER_HH

#include "globals.hh"
#include "G4UImessenger.hh"

class RMGProcessesList;

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIcmdWithABool;
// class RMGUIcmdStepLimit;
// class G4UIcmdWithoutParameter;
class G4UIcmdWithAnInteger;
class RMGProcessesMessenger : public G4UImessenger {

  public:

    RMGProcessesMessenger(RMGProcessesList*);
    ~RMGProcessesMessenger() override;

    RMGProcessesMessenger           (RMGProcessesMessenger const&) = delete;
    RMGProcessesMessenger& operator=(RMGProcessesMessenger const&) = delete;
    RMGProcessesMessenger           (RMGProcessesMessenger&&)      = delete;
    RMGProcessesMessenger& operator=(RMGProcessesMessenger&&)      = delete;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:

    RMGProcessesList*         fProcessesList;
    G4UIdirectory*           fRMGProcessesDir;
    G4UIcmdWithAString*      fRMGProcessesChoiceCmd;
    G4UIcmdWithABool*        fOpticalProcessesCmd;
    G4UIcmdWithABool*        fOpticalOnlyCmd;
    G4UIcmdWithABool*        fLowEnergyProcessesCmd;
    G4UIcmdWithAnInteger*    fLowEnergyProcessesOptionCmd;
    // G4UIcmdWithoutParameter* fGetStepLimitCmd;
    // RMGUIcmdStepLimit*        fStepLimitCmd;
    G4UIcmdWithAnInteger*    fAngCorrCmd;
    G4UIcmdWithABool*        fStoreICLevelData;
};

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

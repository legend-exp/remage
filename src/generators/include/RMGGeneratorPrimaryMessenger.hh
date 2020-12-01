#ifndef _RMGGENERATORPRIMARYMESSENGER_HH_
#define _RMGGENERATORPRIMARYMESSENGER_HH_

#include "globals.hh"
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIcmdWithAnInteger;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithADoubleAndUnit;
class RMGGeneratorPrimary;

class RMGGeneratorPrimaryMessenger : public G4UImessenger {

  public:

    RMGGeneratorPrimaryMessenger(RMGGeneratorPrimary *generator);
    ~RMGGeneratorPrimaryMessenger();

    RMGGeneratorPrimaryMessenger           (RMGGeneratorPrimaryMessenger const&) = delete;
    RMGGeneratorPrimaryMessenger& operator=(RMGGeneratorPrimaryMessenger const&) = delete;
    RMGGeneratorPrimaryMessenger           (RMGGeneratorPrimaryMessenger&&)      = delete;
    RMGGeneratorPrimaryMessenger& operator=(RMGGeneratorPrimaryMessenger&&)      = delete;

    void SetNewValue(G4UIcommand* command, G4String new_values) override;

  private:

    RMGGeneratorPrimary* fGeneratorPrimary;

    G4UIdirectory*             fGeneratorDirectory;
    G4UIcmdWithoutParameter*   fNameCmd;
    G4UIcmdWithAString*        fSelectCmd;
    G4UIcmdWithAString*        fConfineCmd;
    G4UIcmdWithAString*        fVolumeCmd;
    G4UIcmdWithAString*        fVolumeListCmd;
    G4UIcmdWithAnInteger*      fVolumeListFromCmd;
    G4UIcmdWithoutParameter*   fVolumeListClearCmd;
    G4UIcmdWithAnInteger*      fVolumeListAddCmd;
    G4UIcmdWithAnInteger*      fVolumeListToCmd;
    G4UIcmdWithAString*        fVolumeArrayAddCmd;
    G4UIcmdWith3VectorAndUnit* fPositionCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

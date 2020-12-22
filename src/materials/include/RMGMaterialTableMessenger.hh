#ifndef _RMG_MATERIAL_TABLE_MESSENGER_HH_
#define _RMG_MATERIAL_TABLE_MESSENGER_HH_

#include <vector>
#include <memory>

#include "globals.hh"
#include "G4UImessenger.hh"

class RMGMaterialTable;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class RMGMaterialTableMessenger : public G4UImessenger {

  public:

    RMGMaterialTableMessenger(RMGMaterialTable*);
    ~RMGMaterialTableMessenger();

    RMGMaterialTableMessenger           (RMGMaterialTableMessenger const&) = delete;
    RMGMaterialTableMessenger& operator=(RMGMaterialTableMessenger const&) = delete;
    RMGMaterialTableMessenger           (RMGMaterialTableMessenger&&)      = delete;
    RMGMaterialTableMessenger& operator=(RMGMaterialTableMessenger&&)      = delete;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:

    RMGMaterialTable* fMaterialTable;

    std::vector<std::unique_ptr<G4UIdirectory>> fDirectories;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fLArFlatTopPhotonYieldCmd;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fLArSingletLifetimeCmd;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fLArTripletLifetimeCmd;
    std::unique_ptr<G4UIcmdWithADoubleAndUnit> fLArVUVAbsorptionLengthCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

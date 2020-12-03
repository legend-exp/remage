#ifndef _RMG_MATERIAL_TABLE_MESSENGER_HH_
#define _RMG_MATERIAL_TABLE_MESSENGER_HH_

#include "globals.hh"
#include "G4UImessenger.hh"

class RMGMaterialTable;
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

    RMGMaterialTable* fMatTable;

    G4UIdirectory* fGeneralDir;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

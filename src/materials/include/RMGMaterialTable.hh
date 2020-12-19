#ifndef _RMG_MATERIAL_TABLE_HH_
#define _RMG_MATERIAL_TABLE_HH_

#include <memory>
#include <map>

#include "globals.hh"

class G4Material;
class RMGMaterialTableMessenger;
class RMGMaterialTable {

  public:

    RMGMaterialTable();
    ~RMGMaterialTable();

    RMGMaterialTable           (RMGMaterialTable const&) = delete;
    RMGMaterialTable& operator=(RMGMaterialTable const&) = delete;
    RMGMaterialTable           (RMGMaterialTable&&)      = delete;
    RMGMaterialTable& operator=(RMGMaterialTable&&)      = delete;

    static const G4Material* GetMaterial(G4String);

  private:

    void InitializeMaterials();
    void InitializeOpticalProperties();

    static std::map<G4String, G4String> fMaterialAliases;
    std::unique_ptr<RMGMaterialTableMessenger> fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

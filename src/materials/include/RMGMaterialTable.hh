#ifndef _RMG_MATERIAL_TABLE_HH_
#define _RMG_MATERIAL_TABLE_HH_

#include <memory>

#include "globals.hh"

class RMGMaterialTableMessenger;
class RMGMaterialTable {

  public:

    RMGMaterialTable();
    ~RMGMaterialTable();

    RMGMaterialTable           (RMGMaterialTable const&) = delete;
    RMGMaterialTable& operator=(RMGMaterialTable const&) = delete;
    RMGMaterialTable           (RMGMaterialTable&&)      = delete;
    RMGMaterialTable& operator=(RMGMaterialTable&&)      = delete;

  private:

    std::unique_ptr<RMGMaterialTableMessenger> fG4Messenger;

    void InitializeMaterials();
    void InitializeOpticalProperties();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

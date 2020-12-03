#ifndef _RMG_MATERIAL_TABLE_HH_
#define _RMG_MATERIAL_TABLE_HH_

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

    RMGMaterialTableMessenger* fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

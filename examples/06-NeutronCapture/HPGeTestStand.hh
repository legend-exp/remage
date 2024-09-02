#ifndef _HPGE_TEST_STAND_HH_
#define _HPGE_TEST_STAND_HH_

#include "RMGHardware.hh"

class HPGeTestStand : public RMGHardware {

  public:

    G4VPhysicalVolume* DefineGeometry() override;
};

#endif

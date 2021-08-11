#ifndef _DETECTOR_CONSTRUCTION_HH_
#define _DETECTOR_CONSTRUCTION_HH_

#include "RMGManagementDetectorConstruction.hh"

class DetectorConstruction: public RMGManagementDetectorConstruction {

    public:
        G4VPhysicalVolume* DefineGeometry();
};

#endif

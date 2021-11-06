#ifndef _DETECTOR_CONSTRUCTION_HH_
#define _DETECTOR_CONSTRUCTION_HH_

#include "RMGDetectorConstruction.hh"

class DetectorConstruction: public RMGDetectorConstruction {

    public:
        G4VPhysicalVolume* DefineGeometry();
};

#endif

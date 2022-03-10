#include "RMGOpticalDetectorHit.hh"

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4UnitsTable.hh"

#include "RMGLog.hh"

namespace u = CLHEP;

G4ThreadLocal G4Allocator<RMGOpticalDetectorHit>* RMGOpticalDetectorHitAllocator = nullptr;

G4bool RMGOpticalDetectorHit::operator==(const RMGOpticalDetectorHit& right) const {
  return ( this == &right ) ? true : false;
}

void RMGOpticalDetectorHit::Print() {
  RMGLog::Out(RMGLog::debug, "Detector ID: ", fDetectorID);
}

// vim: tabstop=2 shiftwidth=2 expandtab

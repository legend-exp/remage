#ifndef _RMGGENERATORUTIL_HH
#define _RMGGENERATORUTIL_HH

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"

namespace RMGGeneratorUtil {

  bool IsSampleable(std::string g4_solid_type);

  G4ThreeVector rand(const G4VSolid*, bool on_surface=false);

  G4ThreeVector rand(const G4Box*, bool on_surface=false);

  G4ThreeVector rand(const G4Sphere*, bool on_surface=false);

  G4ThreeVector rand(const G4Orb*, bool on_surface=false);

  G4ThreeVector rand(const G4Tubs*, bool on_surface=false);
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMGGENERATORPRIMARYPOSITION_HH_
#define _RMGGENERATORPRIMARYPOSITION_HH_

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"

class RMGVGeneratorPrimaryPosition {

  public:

    inline RMGVGeneratorPrimaryPosition(G4String name) :
      fGeneratorName(name),
      fMaxAttempts(100000) {}

    virtual inline ~RMGVGeneratorPrimaryPosition() {
      if (fG4Messenger) delete fG4Messenger;
    }

    RMGVGeneratorPrimaryPosition           (RMGVGeneratorPrimaryPosition const&) = delete;
    RMGVGeneratorPrimaryPosition& operator=(RMGVGeneratorPrimaryPosition const&) = delete;
    RMGVGeneratorPrimaryPosition           (RMGVGeneratorPrimaryPosition&&)      = delete;
    RMGVGeneratorPrimaryPosition& operator=(RMGVGeneratorPrimaryPosition&&)      = delete;

    virtual inline G4ThreeVector ShootPrimaryPosition() { return kDummyPrimaryPosition; }
    inline void SetMaxAttempts(G4int val) { fMaxAttempts = val; }
    inline G4int GetMaxAttempts() { return fMaxAttempts; }

  protected:

    G4String fGeneratorName;
    G4int fMaxAttempts;
    const G4ThreeVector kDummyPrimaryPosition = G4ThreeVector(0, 0, 0);

    G4UImessenger* fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

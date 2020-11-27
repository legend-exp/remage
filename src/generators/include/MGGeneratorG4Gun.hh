#ifndef _MGGENERATORG4GUN_HH_
#define _MGGENERATORG4GUN_HH_

#include "MGVGenerator.hh"

#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

class G4Event;
class G4ParticleGun;
class MGGeneratorG4GunMessenger;
class MGGeneratorG4Gun : public MGVGenerator {

  public:

    inline MGGeneratorG4Gun() { fParticleGun = new G4ParticleGun(); }
    inline ~MGGeneratorG4Gun() { delete fParticleGun; }

    MGGeneratorG4Gun           (MGGeneratorG4Gun const&) = delete;
    MGGeneratorG4Gun& operator=(MGGeneratorG4Gun const&) = delete;
    MGGeneratorG4Gun           (MGGeneratorG4Gun&&)      = delete;
    MGGeneratorG4Gun& operator=(MGGeneratorG4Gun&&)      = delete;

    inline void GeneratePrimaryVertex(G4Event* event) override {
      fParticleGun->GeneratePrimaryVertex(event);
    }
    inline void SetParticlePosition(G4ThreeVector vec) override {
      fParticleGun->SetParticlePosition(vec);
    }

  private:

    G4ParticleGun* fParticleGun;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

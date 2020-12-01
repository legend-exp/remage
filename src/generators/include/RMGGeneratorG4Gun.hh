#ifndef _RMGGENERATORG4GUN_HH_
#define _RMGGENERATORG4GUN_HH_

#include "RMGVGenerator.hh"

#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

class G4Event;
class G4ParticleGun;
class RMGGeneratorG4GunMessenger;
class RMGGeneratorG4Gun : public RMGVGenerator {

  public:

    inline RMGGeneratorG4Gun() { fParticleGun = new G4ParticleGun(); }
    inline ~RMGGeneratorG4Gun() { delete fParticleGun; }

    RMGGeneratorG4Gun           (RMGGeneratorG4Gun const&) = delete;
    RMGGeneratorG4Gun& operator=(RMGGeneratorG4Gun const&) = delete;
    RMGGeneratorG4Gun           (RMGGeneratorG4Gun&&)      = delete;
    RMGGeneratorG4Gun& operator=(RMGGeneratorG4Gun&&)      = delete;

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

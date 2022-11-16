#ifndef _RMGGENERATORG4GUN_HH_
#define _RMGGENERATORG4GUN_HH_

#include <memory>

#include "RMGVGenerator.hh"

#include "G4ParticleGun.hh"
#include "G4ThreeVector.hh"

class G4Event;
class G4ParticleGun;
class RMGGeneratorG4Gun : public RMGVGenerator {

  public:

    inline RMGGeneratorG4Gun() : RMGVGenerator("G4Gun") {
      fParticleGun = std::unique_ptr<G4ParticleGun>(new G4ParticleGun());
    }
    inline ~RMGGeneratorG4Gun() = default;

    RMGGeneratorG4Gun(RMGGeneratorG4Gun const&) = delete;
    RMGGeneratorG4Gun& operator=(RMGGeneratorG4Gun const&) = delete;
    RMGGeneratorG4Gun(RMGGeneratorG4Gun&&) = delete;
    RMGGeneratorG4Gun& operator=(RMGGeneratorG4Gun&&) = delete;

    inline void GeneratePrimariesKinematics(G4Event* event) override {
      fParticleGun->GeneratePrimaryVertex(event);
    }
    inline void SetParticlePosition(G4ThreeVector vec) override {
      fParticleGun->SetParticlePosition(vec);
    }

  private:

    std::unique_ptr<G4ParticleGun> fParticleGun = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

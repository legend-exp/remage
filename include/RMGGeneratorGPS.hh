#ifndef _RMGGENERATORSPS_HH_
#define _RMGGENERATORSPS_HH_

#include <memory>

#include "G4GeneralParticleSource.hh"
#include "G4ThreeVector.hh"
#include "RMGVGenerator.hh"

class G4Event;
class RMGGeneratorGPS : public RMGVGenerator {

  public:

    inline RMGGeneratorGPS() : RMGVGenerator("GPS") {
      fParticleSource = std::unique_ptr<G4GeneralParticleSource>(new G4GeneralParticleSource());
    }

    inline ~RMGGeneratorGPS() = default;

    inline void GeneratePrimariesKinematics(G4Event* event) override {
      fParticleSource->GeneratePrimaryVertex(event);
    }

    void SetParticlePosition(G4ThreeVector vec) override {
      fParticleSource->GetCurrentSource()->GetPosDist()->SetCentreCoords(vec);
    }

  private:

    std::unique_ptr<G4GeneralParticleSource> fParticleSource = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

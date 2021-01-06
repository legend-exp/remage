#ifndef _RMGGENERATORSPS_HH_
#define _RMGGENERATORSPS_HH_

#include <memory>

#include "G4ThreeVector.hh"
#include "RMGVGenerator.hh"
#include "G4GeneralParticleSource.hh"

class G4Event;
class RMGGeneratorSPS : public RMGVGenerator {

  public:

    inline RMGGeneratorSPS() : RMGVGenerator("SPS") {
      fParticleSource = std::unique_ptr<G4GeneralParticleSource>(new G4GeneralParticleSource());
    }

    inline ~RMGGeneratorSPS() = default;

    inline void GeneratePrimaryVertex(G4Event *event) override {
      fParticleSource->GeneratePrimaryVertex(event);
    }

    void SetParticlePosition(G4ThreeVector vec) override {
      fParticleSource->GetCurrentSource()->GetPosDist()->SetCentreCoords(vec);
    }

  private:

    std::unique_ptr<G4GeneralParticleSource> fParticleSource;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

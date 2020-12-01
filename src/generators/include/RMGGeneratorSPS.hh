#ifndef _RMGGENERATORSPS_HH_
#define _RMGGENERATORSPS_HH_

#include "G4ThreeVector.hh"
#include "RMGVGenerator.hh"
#include "G4GeneralParticleSource.hh"

class G4Event;
class RMGGeneratorSPS : public RMGVGenerator {

  public:

    inline RMGGeneratorSPS() { fParticleSource = new G4GeneralParticleSource(); }
    inline ~RMGGeneratorSPS() { delete fParticleSource; }

    inline void GeneratePrimaryVertex(G4Event *event) override {
      fParticleSource->GeneratePrimaryVertex(event);
    }

    void SetParticlePosition(G4ThreeVector vec) override {
      fParticleSource->GetCurrentSource()->GetPosDist()->SetCentreCoords(vec);
    }

  private:

    G4GeneralParticleSource* fParticleSource;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

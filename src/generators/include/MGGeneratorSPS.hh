#ifndef _MGGENERATORSPS_HH_
#define _MGGENERATORSPS_HH_

#include "G4ThreeVector.hh"
#include "MGVGenerator.hh"
#include "G4GeneralParticleSource.hh"

class G4Event;
class MGGeneratorSPS : public MGVGenerator {

  public:

    inline MGGeneratorSPS() { fParticleSource = new G4GeneralParticleSource(); }
    inline ~MGGeneratorSPS() { delete fParticleSource; }

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

#ifndef _RMG_GENERATOR_DECAY0_HH_
#define _RMG_GENERATOR_DECAY0_HH_

#include <memory>

#include "RMGVGenerator.hh"

#include "G4ThreeVector.hh"

class G4Event;
class RMGGeneratorDecay0Messenger;
class RMGGeneratorDecay0 : public RMGVGenerator {

  public:

    RMGGeneratorDecay0();
    ~RMGGeneratorDecay0();

    RMGGeneratorDecay0           (RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0           (RMGGeneratorDecay0&&)      = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0&&)      = delete;

    void GeneratePrimaryVertex(G4Event*) override;
    void SetParticlePosition(G4ThreeVector) override;

  private:

    std::unique_ptr<RMGGeneratorDecay0Messenger> fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

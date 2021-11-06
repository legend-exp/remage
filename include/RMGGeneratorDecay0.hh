#ifndef _RMG_GENERATOR_DECAY0_HH_
#define _RMG_GENERATOR_DECAY0_HH_

#include <memory>

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

#include "G4ThreeVector.hh"

#include "ProjectInfo.hh"
#if RMG_HAS_BXDECAY0
#include "bxdecay0_g4/primary_generator_action.hh"
#endif

class G4Event;
class RMGGeneratorDecay0 : public RMGVGenerator {

  public:

    RMGGeneratorDecay0(RMGVVertexGenerator* prim_gen);
    RMGGeneratorDecay0() = delete;
    inline ~RMGGeneratorDecay0() = default;

    RMGGeneratorDecay0           (RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0 const&) = delete;
    RMGGeneratorDecay0           (RMGGeneratorDecay0&&)      = delete;
    RMGGeneratorDecay0& operator=(RMGGeneratorDecay0&&)      = delete;

    void GeneratePrimaryVertex(G4Event*) override;
    inline void SetParticlePosition(G4ThreeVector) override {};

  private:

    std::unique_ptr<bxdecay0_g4::PrimaryGeneratorAction> fDecay0G4Generator;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

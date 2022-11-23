#ifndef _RMG_GENERATOR_PRIMARY_POSITION_HH_
#define _RMG_GENERATOR_PRIMARY_POSITION_HH_

#include "ProjectInfo.hh"
#if RMG_HAS_BXDECAY0
#include "bxdecay0_g4/vertex_generator_interface.hh"
#endif

#include <memory>

#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"

class G4Run;
#if RMG_HAS_BXDECAY0
class RMGVVertexGenerator : public bxdecay0_g4::VertexGeneratorInterface {
#else
class RMGVVertexGenerator {
#endif

  public:

    inline RMGVVertexGenerator(std::string name) : fGeneratorName(name) {}

    virtual inline ~RMGVVertexGenerator() = default;

    RMGVVertexGenerator(RMGVVertexGenerator const&) = delete;
    RMGVVertexGenerator& operator=(RMGVVertexGenerator const&) = delete;
    RMGVVertexGenerator(RMGVVertexGenerator&&) = delete;
    RMGVVertexGenerator& operator=(RMGVVertexGenerator&&) = delete;

    virtual inline void BeginOfRunAction(const G4Run*){};
    virtual inline void EndOfRunAction(const G4Run*){};

    virtual inline bool GeneratePrimariesVertex(G4ThreeVector& v) {
      v = kDummyPrimaryPosition;
      return false;
    }
    inline void SetMaxAttempts(int val) { fMaxAttempts = val; }
    inline int GetMaxAttempts() { return fMaxAttempts; }

#if RMG_HAS_BXDECAY0
    inline void ShootVertex(G4ThreeVector& v) { GeneratePrimariesVertex(v); }
#endif

  protected:

    std::string fGeneratorName;
    int fMaxAttempts = 100;
    const G4ThreeVector kDummyPrimaryPosition = G4ThreeVector(0, 0, 0);

    std::unique_ptr<G4UImessenger> fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

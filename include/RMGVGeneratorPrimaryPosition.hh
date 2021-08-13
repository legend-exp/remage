#ifndef _RMG_GENERATOR_PRIMARY_POSITION_HH_
#define _RMG_GENERATOR_PRIMARY_POSITION_HH_

#include "ProjectInfo.hh"
#if RMG_HAS_BXDECAY0
#include "bxdecay0_g4/vertex_generator_interface.hh"
#endif

#include <memory>

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"

#if RMG_HAS_BXDECAY0
class RMGVGeneratorPrimaryPosition : public bxdecay0_g4::VertexGeneratorInterface {
#else
class RMGVGeneratorPrimaryPosition {
#endif

  public:

    inline RMGVGeneratorPrimaryPosition(G4String name) :
      fGeneratorName(name),
      fMaxAttempts(100000) {}

    virtual inline ~RMGVGeneratorPrimaryPosition() = default;

    RMGVGeneratorPrimaryPosition           (RMGVGeneratorPrimaryPosition const&) = delete;
    RMGVGeneratorPrimaryPosition& operator=(RMGVGeneratorPrimaryPosition const&) = delete;
    RMGVGeneratorPrimaryPosition           (RMGVGeneratorPrimaryPosition&&)      = delete;
    RMGVGeneratorPrimaryPosition& operator=(RMGVGeneratorPrimaryPosition&&)      = delete;

    virtual inline G4ThreeVector ShootPrimaryPosition() { return kDummyPrimaryPosition; }
    inline void SetMaxAttempts(G4int val) { fMaxAttempts = val; }
    inline G4int GetMaxAttempts() { return fMaxAttempts; }

#if RMG_HAS_BXDECAY0
    inline void ShootVertex(G4ThreeVector& v) { v = ShootPrimaryPosition(); }
#endif

  protected:

    G4String fGeneratorName;
    G4int fMaxAttempts;
    const G4ThreeVector kDummyPrimaryPosition = G4ThreeVector(0, 0, 0);

    std::unique_ptr<G4UImessenger> fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

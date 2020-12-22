#include "RMGGeneratorDecay0.hh"

#include "RMGGeneratorDecay0Messenger.hh"

RMGGeneratorDecay0::RMGGeneratorDecay0() :
  RMGVGenerator("Decay0") {

  fG4Messenger = std::unique_ptr<RMGGeneratorDecay0Messenger>(new RMGGeneratorDecay0Messenger(this));
}

void RMGGeneratorDecay0::GeneratePrimaryVertex(G4Event*) {
}

void RMGGeneratorDecay0::SetParticlePosition(G4ThreeVector) {
}

// vim: tabstop=2 shiftwidth=2 expandtab

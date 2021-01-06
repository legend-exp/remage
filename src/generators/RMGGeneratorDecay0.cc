#include "RMGGeneratorDecay0.hh"

RMGGeneratorDecay0::RMGGeneratorDecay0() :
  RMGVGenerator("Decay0") {

  fDecay0G4Generator = std::unique_ptr<bxdecay0_g4::PrimaryGeneratorAction>(new bxdecay0_g4::PrimaryGeneratorAction(0));
}

void RMGGeneratorDecay0::GeneratePrimaryVertex(G4Event* event) {
  fDecay0G4Generator->GeneratePrimaries(event);
}

// vim: tabstop=2 shiftwidth=2 expandtab

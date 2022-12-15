#ifndef _RMG_GENERATOR_MUSUN_COSMIC_MUONS_HH_
#define _RMG_GENERATOR_MUSUN_COSMIC_MUONS_HH_

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4VUserPrimaryGeneratorAction.hh"


#include "G4ThreeVector.hh"

#include "ProjectInfo.hh"

namespace u = CLHEP;

class G4Event;
class RMGGeneratorMUSUNCosmicMuons : public RMGVGenerator {

  public:

    RMGGeneratorMUSUNCosmicMuons();
    ~RMGGeneratorMUSUNCosmicMuons() = default;

    RMGGeneratorMUSUNCosmicMuons(RMGGeneratorMUSUNCosmicMuons const&) = delete;
    RMGGeneratorMUSUNCosmicMuons& operator=(RMGGeneratorMUSUNCosmicMuons const&) = delete;
    RMGGeneratorMUSUNCosmicMuons(RMGGeneratorMUSUNCosmicMuons&&) = delete;
    RMGGeneratorMUSUNCosmicMuons& operator=(RMGGeneratorMUSUNCosmicMuons&&) = delete;

    void GeneratePrimaries(G4Event* event);
    void GeneratePrimariesKinematics(G4Event* event) override { this->GeneratePrimaries(event); }
    virtual void SetParticlePosition(G4ThreeVector vec) override{};

    void BeginOfRunAction(const G4Run*);
    inline void EndOfRunAction(const G4Run*) {}

  private:
    void DefineCommands();
    void SetMUSUNFile(G4String fileName);
    std::unique_ptr<G4ParticleGun> fGun = nullptr;
    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;

};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

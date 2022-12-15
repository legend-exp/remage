#include "RMGGeneratorMUSUNCosmicMuons.hh"

#include "RMGVGenerator.hh"
#include "math.h"

#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleMomentum.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

#include "RMGReaderMUSUN.hh"

#include "ProjectInfo.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGTools.hh"


namespace u = CLHEP;

RMGGeneratorMUSUNCosmicMuons::RMGGeneratorMUSUNCosmicMuons() : RMGVGenerator("MUSUNCosmicMuons") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

void RMGGeneratorMUSUNCosmicMuons::BeginOfRunAction(const G4Run*) {

    if(RMGReaderMUSUN::getInstance()->IsEOF())
        RMGLog::Out(RMGLog::fatal, "MUSUN file is at EOF at BeginOfRunAction! File must be empty or incorrect path.");

}

void RMGGeneratorMUSUNCosmicMuons::GeneratePrimaries(G4Event* event) {

  if(RMGReaderMUSUN::getInstance()->IsEOF())
    RMGLog::Out(RMGLog::fatal, "Primary generator trying to generate more muons than lines in the input MUSUN file!");
    
  RMGReaderMUSUN* readerInstance = RMGReaderMUSUN::getInstance();
  MUSUN_OUTPUT data = readerInstance->ReadNextLine();

    G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  if(data.particleID == 10)
    fGun->SetParticleDefinition(theParticleTable->FindParticle("mu-"));
  else
    fGun->SetParticleDefinition(theParticleTable->FindParticle("mu+"));

  RMGLog::OutFormat(RMGLog::debug, "...origin ({:.4g}, {:.4g}, {:.4g}) m", data.x * u::cm / u::m, data.y  * u::cm / u::m, data.z * u::cm / u::m);
  fGun->SetParticlePosition({data.x * u::cm, data.y * u::cm, data.z * u::cm});

  G4ThreeVector d_cart(1, 1, 1);
  d_cart.setTheta(data.theta); // in rad
  d_cart.setPhi(data.phi);     // in rad
  d_cart.setMag(1 * u::m);
  fGun->SetParticleMomentumDirection(d_cart);

  RMGLog::OutFormat(RMGLog::debug, "...direction (θ,φ) = ({:.4g}, {:.4g}) deg",
      data.theta / u::deg, data.phi / u::deg);

  RMGLog::OutFormat(RMGLog::debug, "...energy {:.4g} GeV", data.energy);
  fGun->SetParticleEnergy(data.energy * u::GeV);

  fGun->GeneratePrimaryVertex(event);
}

void RMGGeneratorMUSUNCosmicMuons::SetMUSUNFile(G4String fileName)
{
    RMGReaderMUSUN* readerInstance = RMGReaderMUSUN::getInstance();
    readerInstance->SetInputFile(fileName);
}

void RMGGeneratorMUSUNCosmicMuons::DefineCommands() {

  // NOTE: SetUnit(Category) is not thread-safe

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/MUSUNCosmicMuons/",
      "Commands for controlling the MUSUN µ generator");

  fMessenger->DeclareMethod("SetMUSUNFile", &RMGGeneratorMUSUNCosmicMuons::SetMUSUNFile)
      .SetGuidance("Set the MUSUN input file")
      .SetParameterName("MUSUNFileName", false)
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

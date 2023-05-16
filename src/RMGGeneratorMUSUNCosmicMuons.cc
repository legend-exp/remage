#include "RMGGeneratorMUSUNCosmicMuons.hh"

#include "RMGVGenerator.hh"
#include "math.h"

#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleMomentum.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

#include "ProjectInfo.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGTools.hh"

#include "G4CsvAnalysisReader.hh"

namespace u = CLHEP;

RMGGeneratorMUSUNCosmicMuons::RMGGeneratorMUSUNCosmicMuons() : RMGVGenerator("MUSUNCosmicMuons") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

void RMGGeneratorMUSUNCosmicMuons::BeginOfRunAction(const G4Run*) {

  using G4AnalysisReader = G4CsvAnalysisReader;
  auto analysisReader = G4AnalysisReader::Instance();
  analysisReader->SetVerboseLevel(1);
  analysisReader->SetFileName(fFileName);
  G4int ntupleId = analysisReader->GetNtuple("MUSUN");
  if(ntupleId < 0)
    RMGLog::Out(RMGLog::fatal,
      "MUSUN file not found!");

  analysisReader->SetNtupleIColumn(0,"ID", 	fID	);
  analysisReader->SetNtupleIColumn(0,"type", 	fType	);
  analysisReader->SetNtupleDColumn(0,"Ekin", 	fEkin	);
  analysisReader->SetNtupleDColumn(0,"x", 	fX	);
  analysisReader->SetNtupleDColumn(0,"y", 	fY	);
  analysisReader->SetNtupleDColumn(0,"z", 	fZ	);
  analysisReader->SetNtupleDColumn(0,"theta", fTheta	);
  analysisReader->SetNtupleDColumn(0,"phi", 	fPhi	);
  analysisReader->SetNtupleDColumn(0,"px", 	fPx	);
  analysisReader->SetNtupleDColumn(0,"py", 	fPy	);
  analysisReader->SetNtupleDColumn(0,"pz", 	fPz	);

}


void RMGGeneratorMUSUNCosmicMuons::GeneratePrimaries(G4Event* event) {
  auto analysisReader = G4CsvAnalysisReader::Instance();
  analysisReader->GetNtupleRow();

  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  if (fType == 10) fGun->SetParticleDefinition(theParticleTable->FindParticle("mu-"));
  else fGun->SetParticleDefinition(theParticleTable->FindParticle("mu+"));

  RMGLog::OutFormat(RMGLog::debug, "...origin ({:.4g}, {:.4g}, {:.4g}) m", fX * u::cm / u::m,
      fY * u::cm / u::m, fZ * u::cm / u::m);
  fGun->SetParticlePosition({fX * u::cm, fY * u::cm, fZ * u::cm});

  if(fTheta != 0 && fPhi != 0){
    G4ThreeVector d_cart(1, 1, 1);
    d_cart.setTheta(fTheta); // in rad
    d_cart.setPhi(fPhi);     // in rad
    d_cart.setMag(1 * u::m);
    fGun->SetParticleMomentumDirection(d_cart);
    RMGLog::OutFormat(RMGLog::debug, "...direction (θ,φ) = ({:.4g}, {:.4g}) deg", fTheta / u::deg,
        fPhi / u::deg);
  }
  else{
    G4ThreeVector d_cart(fPx, fPy, fPz);
    fGun->SetParticleMomentumDirection(d_cart);
    RMGLog::OutFormat(RMGLog::debug, "...direction (px,py,pz) = ({:.4g}, {:.4g}, {:.4g}) deg", fPx, fPy, fPz);
  }


  RMGLog::OutFormat(RMGLog::debug, "...energy {:.4g} GeV", fEkin);
  fGun->SetParticleEnergy(fEkin * u::GeV);

  fGun->GeneratePrimaryVertex(event);
}

void RMGGeneratorMUSUNCosmicMuons::SetMUSUNFile(G4String fileName) {
  fFileName = fileName;
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

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

#include <filesystem>
#include <vector>

namespace u = CLHEP;


RMGGeneratorMUSUNCosmicMuons::RMGGeneratorMUSUNCosmicMuons() : RMGVGenerator("MUSUNCosmicMuons") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
}

void RMGGeneratorMUSUNCosmicMuons::PrepareCopy(G4String pathToFile){
  /*
  The working assumption is that the user uses the output directly from MUSUN, i.e. there is no header. 
  To allow proper multiprocessing, we want the file to be read using G4CsvAnalysisReader. 
  To do this, we copy the file to /var/tmp with the appropriate header. 
  To determine the header format, we need to determine the number of columns. 
  */

  // Define fPathToTmpFile
  std::filesystem::path originalFilePath((std::string)pathToFile);
  G4String fileName = originalFilePath.filename().string();
  fPathToTmpFile = fPathToTmpFolder + fileName;

  // Check if the original file exists / the tmp file does not exist
  std::ifstream originalFile(pathToFile);
  if (!originalFile)
    RMGLog::Out(RMGLog::fatal, "MUSUN file not found! Exit.");

  if (std::filesystem::exists((std::string)fPathToTmpFile)) {
    RMGLog::Out(RMGLog::warning, "Temporary file already exists. Deleting it.");
    std::filesystem::remove((std::string)fPathToTmpFile);
  }

  // Counting the number of columns to identify which header to use
  std::string firstLine;
  if (!std::getline(originalFile, firstLine)) {
    std::cerr << "Error: File is empty" << std::endl;
    return;
  }
  std::istringstream iss(firstLine);
  std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>());
  int numColumns = tokens.size();

  // Define header template
  std::string header_template = "#class tools::wcsv::ntuple\n"
                                "#title MUSUN\n"
                                "#separator 11\n"
                                "#vector_separator 10\n"
                                "#column int ID\n"
                                "#column int type\n"
                                "#column double Ekin\n"
                                "#column double x\n"
                                "#column double y\n"
                                "#column double z\n";

  // Based on the number of columns, add additional columns
  if (numColumns == 8) {
      header_template += "#column double theta\n"
                          "#column double phi\n";
  } else if (numColumns == 9) {
      header_template += "#column double px\n"
                          "#column double py\n"
                          "#column double pz\n";
  }
  else
    RMGLog::Out(RMGLog::fatal, "MUSUN format not identified! It has " + to_string(numColumns) + " columns. Exit.");


  // Create a temporary file and write the header
  std::ofstream tmpFile(fPathToTmpFile);
  if (!tmpFile) 
    RMGLog::Out(RMGLog::fatal, "Unable to create temporary file! Exit.");

  tmpFile << header_template;

  // Copy contents from original file to temporary file
  tmpFile << originalFile.rdbuf();

  // Close files
  originalFile.close();
  tmpFile.close();

}

void RMGGeneratorMUSUNCosmicMuons::BeginOfRunAction(const G4Run*) {

  PrepareCopy(fPathToFile);

  using G4AnalysisReader = G4CsvAnalysisReader;
  auto analysisReader = G4AnalysisReader::Instance();
  analysisReader->SetVerboseLevel(1);
  analysisReader->SetFileName(fPathToTmpFile);
  G4int ntupleId = analysisReader->GetNtuple("MUSUN");
  if (ntupleId < 0) RMGLog::Out(RMGLog::fatal, "MUSUN file not found!");

  analysisReader->SetNtupleIColumn(0, "ID", fID);
  analysisReader->SetNtupleIColumn(0, "type", fType);
  analysisReader->SetNtupleDColumn(0, "Ekin", fEkin);
  analysisReader->SetNtupleDColumn(0, "x", fX);
  analysisReader->SetNtupleDColumn(0, "y", fY);
  analysisReader->SetNtupleDColumn(0, "z", fZ);
  analysisReader->SetNtupleDColumn(0, "theta", fTheta);
  analysisReader->SetNtupleDColumn(0, "phi", fPhi);
  analysisReader->SetNtupleDColumn(0, "px", fPx);
  analysisReader->SetNtupleDColumn(0, "py", fPy);
  analysisReader->SetNtupleDColumn(0, "pz", fPz);
}

void RMGGeneratorMUSUNCosmicMuons::EndOfRunAction(const G4Run*) {
  std::filesystem::remove((std::string)fPathToTmpFile);
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

  if (fTheta != 0 && fPhi != 0) {
    G4ThreeVector d_cart(1, 1, 1);
    d_cart.setTheta(fTheta); // in rad
    d_cart.setPhi(fPhi);     // in rad
    d_cart.setMag(1 * u::m);
    fGun->SetParticleMomentumDirection(d_cart);
    RMGLog::OutFormat(RMGLog::debug, "...direction (θ,φ) = ({:.4g}, {:.4g}) deg", fTheta / u::deg,
        fPhi / u::deg);
  } else {
    G4ThreeVector d_cart(fPx, fPy, fPz);
    fGun->SetParticleMomentumDirection(d_cart);
    RMGLog::OutFormat(RMGLog::debug, "...direction (px,py,pz) = ({:.4g}, {:.4g}, {:.4g}) deg", fPx,
        fPy, fPz);
  }


  RMGLog::OutFormat(RMGLog::debug, "...energy {:.4g} GeV", fEkin);
  fGun->SetParticleEnergy(fEkin * u::GeV);

  fGun->GeneratePrimaryVertex(event);
}

void RMGGeneratorMUSUNCosmicMuons::SetMUSUNFile(G4String pathToFile) { fPathToFile = pathToFile; }

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

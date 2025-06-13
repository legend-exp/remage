// Copyright (C) 2024 Moritz Neuberger
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
#include "RMGGeneratorMUSUNCosmicMuons.hh"

#include <cmath>
#include <filesystem>
#include <iterator>
#include <vector>

#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGVGenerator.hh"

namespace u = CLHEP;

RMGAnalysisReader* RMGGeneratorMUSUNCosmicMuons::fAnalysisReader = new RMGAnalysisReader();
RMGGeneratorMUSUNCosmicMuons_Data* RMGGeneratorMUSUNCosmicMuons::fInputData = nullptr;

RMGGeneratorMUSUNCosmicMuons::RMGGeneratorMUSUNCosmicMuons() : RMGVGenerator("MUSUNCosmicMuons") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
  fPathToTmpFolder = std::filesystem::temp_directory_path();
}

void RMGGeneratorMUSUNCosmicMuons::PrepareCopy(std::string pathToFile) {
  /*
  The working assumption is that the user uses the output directly from MUSUN, i.e. there is no header.
  To allow proper multiprocessing, we want the file to be read using G4CsvAnalysisReader.
  To do this, we copy the file to /var/tmp with the appropriate header.
  To determine the header format, we need to determine the number of columns.
  */

  // Define fPathToTmpFile
  std::filesystem::path originalFilePath(pathToFile);
  std::filesystem::path fileName = originalFilePath.filename();
  fPathToTmpFile = fPathToTmpFolder / fileName;

  // Check if the original file exists / the tmp file does not exist
  std::ifstream originalFile(pathToFile);
  if (!originalFile) RMGLog::Out(RMGLog::fatal, "MUSUN file ", pathToFile, " not found! Exit.");

  if (std::filesystem::exists(fPathToTmpFile)) {
    RMGLog::Out(RMGLog::warning, "Temporary file already exists. Deleting it.");
    std::filesystem::remove(fPathToTmpFile);
  }

  // Counting the number of columns to identify which header to use
  std::string firstLine;
  if (!std::getline(originalFile, firstLine)) {
    RMGLog::Out(RMGLog::error, "Error: File is empty");
    return;
  }
  std::istringstream iss(firstLine);
  std::vector<std::string> tokens(
      std::istream_iterator<std::string>{iss},
      std::istream_iterator<std::string>()
  );
  size_t numColumns = tokens.size();

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
  } else
    RMGLog::Out(
        RMGLog::fatal,
        "MUSUN format not identified! It has " + to_string(numColumns) + " columns. Exit."
    );


  // Create a temporary file and write the header
  std::ofstream tmpFile(fPathToTmpFile);
  if (!tmpFile) RMGLog::Out(RMGLog::fatal, "Unable to create temporary file! Exit.");

  tmpFile << header_template;

  // Copy contents from original file to temporary file
  tmpFile << firstLine << std::endl;
  tmpFile << originalFile.rdbuf();

  // Close files
  originalFile.close();
  tmpFile.close();
}

void RMGGeneratorMUSUNCosmicMuons::BeginOfRunAction(const G4Run*) {

  if (!fInputData) {
    if (!G4Threading::IsMasterThread()) {
      RMGLog::OutDev(RMGLog::fatal, "on worker, but data not initialized");
    }

    // include in lock
    auto lock = fAnalysisReader->GetLock();
    PrepareCopy(fPathToFile);

    auto reader = fAnalysisReader->OpenFile(fPathToTmpFile, "", "MUSUN", std::move(lock), "csv");
    if (!reader) RMGLog::Out(RMGLog::fatal, "Temp MUSUN file not found! Exit.");

    fInputData = new RMGGeneratorMUSUNCosmicMuons_Data;
    reader.SetNtupleIColumn("ID", (fInputData->fID));
    reader.SetNtupleIColumn("type", (fInputData->fType));
    reader.SetNtupleDColumn("Ekin", (fInputData->fEkin));
    reader.SetNtupleDColumn("x", (fInputData->fX));
    reader.SetNtupleDColumn("y", (fInputData->fY));
    reader.SetNtupleDColumn("z", (fInputData->fZ));
    reader.SetNtupleDColumn("theta", (fInputData->fTheta));
    reader.SetNtupleDColumn("phi", (fInputData->fPhi));
    reader.SetNtupleDColumn("px", (fInputData->fPx));
    reader.SetNtupleDColumn("py", (fInputData->fPy));
    reader.SetNtupleDColumn("pz", (fInputData->fPz));
  }
}

void RMGGeneratorMUSUNCosmicMuons::EndOfRunAction(const G4Run*) {
  if (!G4Threading::IsMasterThread()) return;

  auto reader = fAnalysisReader->GetLockedReader();
  if (reader) { std::filesystem::remove(fPathToTmpFile); }
}


void RMGGeneratorMUSUNCosmicMuons::GeneratePrimaries(G4Event* event) {

  auto reader = fAnalysisReader->GetLockedReader();

  if (!reader) {
    RMGLog::Out(RMGLog::error, "MUSUN ntuple could not be found in input file!");
    return;
  }

  if (!reader.GetNtupleRow()) {
    RMGLog::Out(RMGLog::error, "no more input rows in MUSUN file!");
    return;
  }

  // copy data and end critical section.
  RMGGeneratorMUSUNCosmicMuons_Data input_data = *fInputData;
  reader.unlock();

  auto theParticleTable = G4ParticleTable::GetParticleTable();
  if (input_data.fType == 10) fGun->SetParticleDefinition(theParticleTable->FindParticle("mu-"));
  else fGun->SetParticleDefinition(theParticleTable->FindParticle("mu+"));

  RMGLog::OutFormat(
      RMGLog::debug,
      "...origin ({:.4g}, {:.4g}, {:.4g}) m",
      input_data.fX * u::cm / u::m,
      input_data.fY * u::cm / u::m,
      input_data.fZ * u::cm / u::m
  );
  fGun->SetParticlePosition({input_data.fX * u::cm, input_data.fY * u::cm, input_data.fZ * u::cm});

  if (input_data.fTheta != 0 && input_data.fPhi != 0) {
    G4ThreeVector d_cart(1, 1, 1);
    d_cart.setTheta(input_data.fTheta); // in rad
    d_cart.setPhi(input_data.fPhi);     // in rad
    d_cart.setMag(1 * u::m);
    fGun->SetParticleMomentumDirection(d_cart);
    RMGLog::OutFormat(
        RMGLog::debug,
        "...direction (θ,φ) = ({:.4g}, {:.4g}) deg",
        input_data.fTheta / u::deg,
        input_data.fPhi / u::deg
    );
  } else {
    G4ThreeVector d_cart(input_data.fPx, input_data.fPy, input_data.fPz);
    fGun->SetParticleMomentumDirection(d_cart);
    RMGLog::OutFormat(
        RMGLog::debug,
        "...direction (px,py,pz) = ({:.4g}, {:.4g}, {:.4g}) deg",
        input_data.fPx,
        input_data.fPy,
        input_data.fPz
    );
  }

  RMGLog::OutFormat(RMGLog::debug, "...energy {:.4g} GeV", input_data.fEkin);
  fGun->SetParticleEnergy(input_data.fEkin * u::GeV);

  fGun->GeneratePrimaryVertex(event);
}

void RMGGeneratorMUSUNCosmicMuons::SetMUSUNFile(G4String pathToFile) { fPathToFile = pathToFile; }

void RMGGeneratorMUSUNCosmicMuons::DefineCommands() {

  // NOTE: SetUnit(Category) is not thread-safe

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Generator/MUSUNCosmicMuons/",
      "Commands for controlling the MUSUN µ generator"
  );

  fMessenger->DeclareMethod("MUSUNFile", &RMGGeneratorMUSUNCosmicMuons::SetMUSUNFile)
      .SetGuidance("Set the MUSUN input file")
      .SetParameterName("MUSUNFileName", false)
      .SetToBeBroadcasted(true)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

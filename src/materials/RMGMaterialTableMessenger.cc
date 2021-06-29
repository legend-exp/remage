#include "RMGMaterialTableMessenger.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcommand.hh"

#include "RMGTools.hh"
#include "RMGMaterialTable.hh"
#include "RMGLog.hh"

RMGMaterialTableMessenger::RMGMaterialTableMessenger(RMGMaterialTable* table) :
  fMaterialTable(table) {

  G4String directory = "/RMG/Materials";

  fDirectories.emplace_back(new G4UIdirectory(directory));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/LAr").c_str()));

  new G4UnitDefinition("1/eV", "1/eV", "ScintillationYield", 1./CLHEP::eV);
  new G4UnitDefinition("1/keV", "1/keV", "ScintillationYield", 1./CLHEP::keV);
  new G4UnitDefinition("1/MeV", "1/MeV", "ScintillationYield", 1./CLHEP::MeV);

  fLArFlatTopPhotonYieldCmd = RMGTools::MakeG4UIcmdWithANumberAndUnit<G4UIcmdWithADoubleAndUnit>(
      directory + "/LAr/FlatTopPhotonYield", this, "ScintillationYield", "Y", "Y > 0");

  fLArSingletLifetimeCmd = RMGTools::MakeG4UIcmdWithANumberAndUnit<G4UIcmdWithADoubleAndUnit>(
      directory + "/LAr/SingletLifetime", this, "Time", "T", "T > 0");

  fLArTripletLifetimeCmd = RMGTools::MakeG4UIcmdWithANumberAndUnit<G4UIcmdWithADoubleAndUnit>(
      directory + "/LAr/TripletLifetime", this, "Time", "T", "T > 0");

  fLArVUVAbsorptionLengthCmd = RMGTools::MakeG4UIcmdWithANumberAndUnit<G4UIcmdWithADoubleAndUnit>(
      directory + "/LAr/VUVAbsorptionLength", this, "Length", "L", "L > 0");
}

void RMGMaterialTableMessenger::SetNewValue(G4UIcommand* cmd, G4String new_values) {

  if (cmd == fLArFlatTopPhotonYieldCmd.get()) {
    fMaterialTable->SetLArFlatTopPhotonYield(fLArFlatTopPhotonYieldCmd->GetNewDoubleValue(new_values));
  }
  else if (cmd == fLArSingletLifetimeCmd.get()) {
    fMaterialTable->SetLArSingletLifetime(fLArSingletLifetimeCmd->GetNewDoubleValue(new_values));
  }
  else if (cmd == fLArTripletLifetimeCmd.get()) {
    fMaterialTable->SetLArTripletLifetime(fLArTripletLifetimeCmd->GetNewDoubleValue(new_values));
  }
  else if (cmd == fLArVUVAbsorptionLengthCmd.get()) {
    fMaterialTable->SetLArVUVAbsorptionLength(fLArVUVAbsorptionLengthCmd->GetNewDoubleValue(new_values));
  }
  else {
    RMGLog::Out(RMGLog::fatal, "Action of command '", cmd->GetTitle(), "' not implemented");
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

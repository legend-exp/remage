#include <sstream>

#include "G4Tokenizer.hh"
#include "G4UnitsTable.hh"

#include "RMGUIcmdStepLimit.hh"

RMGUIcmdStepLimit::RMGUIcmdStepLimit(std::string command_path, G4UImessenger* aMessenger):
  G4UIcommand(G4String(command_path), aMessenger) {

  auto particle_par = new G4UIparameter('s');
  this->SetParameter(particle_par);
  particle_par->SetParameterName("particle name");

  auto volume_par = new G4UIparameter('s');
  this->SetParameter(volume_par);
  volume_par->SetParameterName("physical volume name");

  auto step_val_par = new G4UIparameter('d');
  this->SetParameter(step_val_par);
  step_val_par->SetParameterName("step value");

  auto step_unit_par = new G4UIparameter('s');
  this->SetParameter(step_unit_par);
  step_unit_par->SetParameterName("step unit");

  this->AvailableForStates(G4State_PreInit);

  this->SetGuidance("Set step limit for [particle] in [physical volume name].");
}

double RMGUIcmdStepLimit::GetStepSize(std::string par_string) {

  std::string particle, volume, units;
  double value;

  std::istringstream is(par_string);
  is >> particle >> volume >> value >> units;

  return value*this->ValueOf(G4String(units));
}

std::string RMGUIcmdStepLimit::GetParticleName(std::string par_string) {

  std::string particle, volume, units;
  double value;

  std::istringstream is(par_string);
  is >> particle >> volume >> value >> units;

  return particle;
}

std::string RMGUIcmdStepLimit::GetVolumeName(std::string par_string) {

  std::string particle, volume, units;
  double value;

  std::istringstream is(par_string);
  is >> particle >> volume >> value >> units;

  return volume;
}

// vim: tabstop=2 shiftwidth=2 expandtab

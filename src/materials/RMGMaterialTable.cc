#include "RMGMaterialTable.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "RMGMaterialTableMessenger.hh"
#include "RMGLog.hh"

RMGMaterialTable::RMGMaterialTable() {
  fG4Messenger = std::unique_ptr<RMGMaterialTableMessenger>(new RMGMaterialTableMessenger(this));

  this->InitializeMaterials();
  this->InitializeOpticalProperties();
}

const G4Material* RMGMaterialTable::GetMaterial(G4String name) {

  auto man = G4NistManager::Instance();
  if (RMGLog::GetLogLevelScreen() < RMGLog::detail) man->SetVerbose(1);

  if (fMaterialAliases.find(name) != fMaterialAliases.end()) {
    return man->FindOrBuildMaterial(fMaterialAliases[name]);
  }
  else return G4Material::GetMaterial("RMG_" + name);
}

void RMGMaterialTable::InitializeMaterials() {

  fMaterialAliases = {
    {"Air",            "G4_AIR"},
    {"Brass",          "G4_BRASS"},
    {"Bronze",         "G4_BRONZE"},
    {"Concrete",       "G4_CONCRETE"},
    {"StainlessSteel", "G4_STAINLESS-STEEL"},
    {"Vacuum",         "G4_Galactic"},
    {"Water",          "G4_WATER"},
    {"Kapton",         "G4_KAPTON"}
  };

  auto man = G4NistManager::Instance();
  if (RMGLog::GetLogLevelScreen() < RMGLog::detail) man->SetVerbose(1);

  std::vector<G4String> elements;
  std::vector<G4int> n_atoms;
  std::vector<G4double> mass_fraction;
  G4double density; // g/cm3
  G4bool isotopes;
  G4State state;
  G4double temperature;
  G4double pressure;

  man->ConstructNewMaterial("RMG_LAr",
      elements    = {"Ar"},
      n_atoms     = {1},
      density     = 1.396,
      isotopes    = true,
      state       = G4State::kStateLiquid,
      temperature = 87*CLHEP::kelvin,
      pressure    = CLHEP::STP_Pressure);

}

void RMGMaterialTable::InitializeOpticalProperties() {
}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "HPGeTestStand.hh"

#include "G4Box.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"
#include "G4PVPlacement.hh"
#include "G4UnitsTable.hh"

namespace u = CLHEP;

G4VPhysicalVolume* HPGeTestStand::DefineGeometry() {

  // just used for "named arguments" further down.
  G4State state = G4State::kStateSolid;
  std::string name, symbol;
  std::vector<std::string> elements;
  std::vector<double> mass_fraction;
  double density = 0;
  double temperature = 0;
  double pressure = 0;
  double abundance = 0;
  int n_isotopes = 0;
  int n_components = 0;
  int n_atoms = 0;

  auto man = G4NistManager::Instance();

  // define enriched germanium
  auto Ge70 = new G4Isotope(name = "Ge70", 32, 70, 69.92 * u::g / u::mole);
  auto Ge72 = new G4Isotope(name = "Ge72", 32, 72, 71.92 * u::g / u::mole);
  auto Ge73 = new G4Isotope(name = "Ge73", 32, 73, 73.00 * u::g / u::mole);
  auto Ge74 = new G4Isotope(name = "Ge74", 32, 74, 74.00 * u::g / u::mole);
  auto Ge76 = new G4Isotope(name = "Ge76", 32, 76, 76.00 * u::g / u::mole);

  auto el_enr_ge = new G4Element(name = "EnrichedGermanium", symbol = "EnrGe", n_isotopes = 5);
  el_enr_ge->AddIsotope(Ge70, abundance = 0.0 * u::perCent);
  el_enr_ge->AddIsotope(Ge72, abundance = 0.1 * u::perCent);
  el_enr_ge->AddIsotope(Ge73, abundance = 0.2 * u::perCent);
  el_enr_ge->AddIsotope(Ge74, abundance = 13.1 * u::perCent);
  el_enr_ge->AddIsotope(Ge76, abundance = 86.6 * u::perCent);

  auto LAr = man->FindOrBuildMaterial("G4_lAr");
  auto water = man->FindOrBuildMaterial("G4_WATER");

  density = 3.01 * u::g / u::cm3;

  auto elGd = new G4Element("Gadolinium", "Gd", 64, 157.25 * u::g / u::mole);
  auto elS = new G4Element("Sulfur", "S", 16., 32.066 * u::g / u::mole);
  auto O = new G4Element("Oxygen", "O", 8., 16.00 * u::g / u::mole);

  auto gadoliniumSulfate = new G4Material("GadoliniumSulfate", density, 3); // Gd2(SO4)3
  gadoliniumSulfate->AddElement(elGd, 2);
  gadoliniumSulfate->AddElement(elS, 3);
  gadoliniumSulfate->AddElement(O, 12);

  auto Gdwater = new G4Material("GdLoadedWater", 1.000000 * u::g / u::cm3, 2);
  Gdwater->AddMaterial(water, 1. - 0.002);
  Gdwater->AddMaterial(gadoliniumSulfate, 0.002);


  // Place Volumes
  // ooooooooooooooooooooooOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooooooooooooooooooo

  auto mat_enr_ge = new G4Material(
      "CryogenicEnrichedGermanium",
      density = 5.56 * u::g / (u::cm * u::cm * u::cm),
      n_components = 1,
      state = G4State::kStateSolid,
      temperature = LAr->GetTemperature(),
      pressure = LAr->GetPressure()
  );

  mat_enr_ge->AddElement(el_enr_ge, n_atoms = 1);

  auto world_s = new G4Box("WoldWater", 0.5 * u::m, 0.5 * u::m, 0.5 * u::m);

  auto world_l = new G4LogicalVolume(world_s, Gdwater, "WoldWater");

  auto world_p = new G4PVPlacement(nullptr, G4ThreeVector(), world_l, "World", nullptr, false, 0);

  auto hpge_s = new G4Box("HPGe", 10 * u::cm, 10 * u::cm, 10 * u::cm);


  auto hpge1_l = new G4LogicalVolume(
      hpge_s,
      G4Material::GetMaterial("CryogenicEnrichedGermanium"),
      "HPGe1"
  );
  auto hpge2_l = new G4LogicalVolume(
      hpge_s,
      G4Material::GetMaterial("CryogenicEnrichedGermanium"),
      "HPGe2"
  );
  auto hpge3_l = new G4LogicalVolume(
      hpge_s,
      G4Material::GetMaterial("CryogenicEnrichedGermanium"),
      "HPGe3"
  );
  auto hpge4_l = new G4LogicalVolume(
      hpge_s,
      G4Material::GetMaterial("CryogenicEnrichedGermanium"),
      "HPGe4"
  );


  // Place HPGe Detectors
  auto spacing = 10.5 * u::cm;
  new G4PVPlacement(nullptr, G4ThreeVector(+spacing, +spacing, 0), hpge1_l, "HPGe1", world_l, false, 0);
  new G4PVPlacement(nullptr, G4ThreeVector(+spacing, -spacing, 0), hpge2_l, "HPGe2", world_l, false, 0);
  new G4PVPlacement(nullptr, G4ThreeVector(-spacing, +spacing, 0), hpge3_l, "HPGe3", world_l, false, 0);
  new G4PVPlacement(nullptr, G4ThreeVector(-spacing, -spacing, 0), hpge4_l, "HPGe4", world_l, false, 0);


  return world_p;
}

// vim: tabstop=2 textwidth=2 expandtab

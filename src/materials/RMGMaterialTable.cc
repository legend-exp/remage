#include "RMGMaterialTable.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4MaterialPropertiesTable.hh"

#include "RMGLog.hh"
#include "RMGManagementDetectorConstruction.hh"

std::map<G4String, G4String> RMGMaterialTable::fMaterialAliases = {};
std::map<RMGMaterialTable::BathMaterial, RMGMaterialTable::PropertiesAtTemperature> RMGMaterialTable::fPropertiesAtTemperatureTable = {};

RMGMaterialTable::RMGMaterialTable() {

  this->DefineCommands();

  // default LAr optical properties
  fLArProperties.flat_top_photon_yield = 51 *1./CLHEP::keV;
  fLArProperties.singlet_lifetime = 5.95 *CLHEP::ns;
  fLArProperties.triplet_lifetime = 1 *CLHEP::us;
  fLArProperties.vuv_absorption_length = 30 *CLHEP::cm;

  fPropertiesAtTemperatureTable.emplace(BathMaterial::kNone,
      PropertiesAtTemperature("",
        5.32 *CLHEP::g/CLHEP::cm3,
        5.54 *CLHEP::g/CLHEP::cm3));

  fPropertiesAtTemperatureTable.emplace(BathMaterial::kLiquidArgon,
      PropertiesAtTemperature("LiquidArgon",
        5.34 *CLHEP::g/CLHEP::cm3,
        5.56 *CLHEP::g/CLHEP::cm3));

  this->InitializeMaterials();
  this->InitializeOpticalProperties();
}

G4Material* RMGMaterialTable::GetMaterial(G4String name) {

  auto man = G4NistManager::Instance();
  if (RMGLog::GetLogLevelScreen() < RMGLog::detail) man->SetVerbose(1);

  if (fMaterialAliases.find(name) != fMaterialAliases.end()) {
    return man->FindOrBuildMaterial(fMaterialAliases[name]);
  }
  else return G4Material::GetMaterial("RMG_" + name);
}

G4Material* RMGMaterialTable::GetMaterial(RMGMaterialTable::BathMaterial val) {
  if (val == BathMaterial::kNone) return nullptr;
  else {
    return RMGMaterialTable::GetMaterial(fPropertiesAtTemperatureTable[val].g4_name);
  }
}

void RMGMaterialTable::InitializeMaterials() {

  fMaterialAliases = {
    {"Air",            "G4_AIR"},
    {"Brass",          "G4_BRASS"},
    {"Bronze",         "G4_BRONZE"},
    {"Concrete",       "G4_CONCRETE"},
    {"Germanium",      "G4_Ge"}, // with natural istopic composition
    {"Kapton",         "G4_KAPTON"},
    {"LiquidArgon",    "G4_lAr"},
    {"LiquidNitrogen", "G4_lN2"},
    {"LiquidXenon",    "G4_lXe"},
    {"Nylon",          "G4_NYLON-6-6"},
    {"Teflon",         "G4_TEFLON"},
    {"StainlessSteel", "G4_STAINLESS-STEEL"},
    {"Vacuum",         "G4_Galactic"},
    {"Water",          "G4_WATER"}
  };

  auto man = G4NistManager::Instance();
  if (RMGLog::GetLogLevelScreen() < RMGLog::detail) man->SetVerbose(1);

  G4String name, symbol;
  std::vector<G4String> elements;
  std::vector<G4int> n_atoms;
  std::vector<G4double> mass_fraction;
  G4double density; // g/cm3
  G4bool isotopes;
  G4State state;
  G4double temperature;
  G4double pressure;
  G4double abundance;
  G4int n_isotopes;

  // define enriched germanium
  auto Ge70 = new G4Isotope(name="Ge70", 32, 70, 69.92 *CLHEP::g/CLHEP::mole);
  auto Ge72 = new G4Isotope(name="Ge72", 32, 72, 71.92 *CLHEP::g/CLHEP::mole);
  auto Ge73 = new G4Isotope(name="Ge73", 32, 73, 73.00 *CLHEP::g/CLHEP::mole);
  auto Ge74 = new G4Isotope(name="Ge74", 32, 74, 74.00 *CLHEP::g/CLHEP::mole);
  auto Ge76 = new G4Isotope(name="Ge76", 32, 76, 76.00 *CLHEP::g/CLHEP::mole);

  auto elGeEnr = new G4Element(name="EnrichedGermanium", symbol="EnrGe", n_isotopes=5);
  elGeEnr->AddIsotope(Ge70, abundance= 0.0 *CLHEP::perCent);
  elGeEnr->AddIsotope(Ge72, abundance= 0.1 *CLHEP::perCent);
  elGeEnr->AddIsotope(Ge73, abundance= 0.2 *CLHEP::perCent);
  elGeEnr->AddIsotope(Ge74, abundance=13.1 *CLHEP::perCent);
  elGeEnr->AddIsotope(Ge76, abundance=86.6 *CLHEP::perCent);

  man->ConstructNewMaterial("RMG_EnrichedGermanium",
      elements    = {"EnrGe"},
      n_atoms     = {1},
      density     = 5.54,
      isotopes    = true,
      state       = G4State::kStateSolid,
      temperature = CLHEP::STP_Temperature,
      pressure    = CLHEP::STP_Pressure);

  auto bath_material = RMGManagementDetectorConstruction::GetBathMaterial();

  // new germanium with cryogenic properties
  man->BuildMaterialWithNewDensity("RMG_CryogenicGermanium",
      "G4_Ge",
      density = fPropertiesAtTemperatureTable[bath_material].germanium_density,
      RMGMaterialTable::GetMaterial(bath_material)->GetTemperature(),
      RMGMaterialTable::GetMaterial(bath_material)->GetPressure());

  // new germanium with cryogenic properties
  man->BuildMaterialWithNewDensity("RMG_CryogenicEnrichedGermanium",
      "RMG_EnrichedGermanium",
      density = fPropertiesAtTemperatureTable[bath_material].enriched_germanium_density,
      RMGMaterialTable::GetMaterial(bath_material)->GetTemperature(),
      RMGMaterialTable::GetMaterial(bath_material)->GetPressure());
}

void RMGMaterialTable::InitializeOpticalProperties() {
  this->InitializeLArOpticalProperties();
}

void RMGMaterialTable::InitializeLArOpticalProperties() {

  // helpers
  auto to_energy     = [](G4double lambda) { return CLHEP::h_Planck * CLHEP::c_light / lambda; };
  auto to_wavelength = to_energy;

  auto lar_mpt = new G4MaterialPropertiesTable();

  /* lar_dielectric_constant(wavelenght)
   *
   * Calculates the dielectric constant of LAr with the Bideau-Sellmeier formula.
   * See: A. Bideau-Mehu et al., "Measurement of refractive indices of Ne, Ar,
   * Kr and Xe ...", J. Quant. Spectrosc. Radiat. Transfer, Vol. 25 (1981), 395
   */

  auto lar_dielectric_const = [](G4double lambda) {
    if (lambda < 110*CLHEP::nm) return 1.0e4; // lambda MUST be > 110.0 nm

    G4double eps = lambda / CLHEP::um; // switch to micrometers
    eps = 1.0 / (eps * eps);    // 1 / (lambda)^2
    eps = 1.2055e-2 * ( 0.2075 / (91.012 - eps) +
                        0.0415 / (87.892 - eps) +
                        4.3330 / (214.02 - eps) );
    eps *= (8./12); // Bideau-Sellmeier -> Clausius-Mossotti
    G4double lar_rho = 1.396 * CLHEP::g/CLHEP::cm3;
    G4double gar_rho = 1.66e-03 * CLHEP::g/CLHEP::cm3;
    eps *= (lar_rho / gar_rho); // density correction (Ar gas -> LAr liquid)

    if (eps < 0.0 || eps > 0.999999) return 4.0e6;
    else return (1. + 2. * eps) / (1. - eps); // solve Clausius-Mossotti
  };

    /* lar_rayleigh_length(wavelenght, temperature)
   *
   * Calculates the Rayleigh scattering length using equations given in
   * G. M. Seidel at al., "Rayleigh scattering in rare-gas liquids",
   * arXiv:hep-ex/0111054 v2 22 Apr 2002
   *
   * This calculation leads to about 70cm length at 128nm, but keep in mind that
   * the value changes drastically out the scintillation peak (39cm at 121nm and
   * 110cm at 135nm).
   */

  auto lar_rayleigh_length = [&lar_dielectric_const](G4double lambda, G4double temp) {
    G4double dyne = 1.0e-5 * CLHEP::newton;
    const G4double lar_kt = 2.18e-10 * CLHEP::cm2/dyne;           // LAr isothermal compressibility
    const G4double k = 1.380658e-23 * CLHEP::joule/CLHEP::kelvin; // the Boltzmann constant
    G4double h;
    h = lar_dielectric_const(lambda);
    if (h < 1.00000001) h = 1.00000001;     // just a precaution
    h = (h - 1.0) * (h + 2.0);              // the "dielectric constant" dependance
    h *= h;                                 // take the square
    h *= lar_kt * temp * k;                 // compressibility * temp * Boltzmann constant
    h /= lambda * lambda * lambda * lambda; // (lambda)^4
    h *= 9.18704494231105429;               // (2 * Pi / 3)^3
    if ( h < (1.0 / (10.0 * CLHEP::km)) ) h = 1.0 / (10.0 * CLHEP::km); // just a precaution
    if ( h > (1.0 / (0.1 * CLHEP::nm)) ) h = 1.0 / (0.1 * CLHEP::nm); // just a precaution

    return 1. / h;
  };

  /* lar_scint_spectrum(wavelenght)
   *
   * The formula describes only the peak at 128 nm, not the whole spectrum.
   * However, the intensity drops by several orders of magnitude out of the
   * peak, so it's a good approximation to consider just the peak.
   *
   * This paper http://iopscience.iop.org/article/10.1209/0295-5075/91/62002/meta
   * shows the whole spectrum in fig. 1 & 2
   */

  auto lar_scint_spectrum = [](G4double kk) {
    return std::exp(-0.5*((kk-128*CLHEP::nm)/(2.929*CLHEP::nm))*((kk-128*CLHEP::nm)/(2.929*CLHEP::nm)));
  };

  // sample 100 points in 128nm +- 15nm
  auto E1 = to_energy(143*CLHEP::nm);
  auto E2 = to_energy(113*CLHEP::nm);
  auto dE = (E2 - E1)/100;

  for (int e = E1; e <= E2; e += dE) {
    lar_mpt->AddEntry("FASTCOMPONENT", e, lar_scint_spectrum(to_wavelength(e)));
    lar_mpt->AddEntry("SLOWCOMPONENT", e, lar_scint_spectrum(to_wavelength(e)));
  }
  // make sure it is zero at the edges
  lar_mpt->AddEntry("FASTCOMPONENT", E1-dE, 0);
  lar_mpt->AddEntry("FASTCOMPONENT", E2+dE, 0);

  /* Scintillation yield (mean energy to produce a UV photon)
   *
   * depends on the nature of the impinging particles, the field configuration
   * and the quencher impurities. We set here just a reference value [1], that
   * probably does not represent the reality of GERDA. Bjorn Lehnert calculated
   * a value of 8000 / MeV [2], Birgit Schneider a value of 1600 / MeV [3]. This
   * value is assigned to electrons, gammas and all the particles except for alphas
   * and nuclear recoils. For those we implement the measurements in [1] by
   * declaring additional Scintillation processes in the physics list.
   *
   * A macro command is available to adjust the PY for electrons and gammas:
   *  /RMG/Materials/LAr/FlatTopPhotonYield <value> 1/keV
   *
   * WArP data [1]
   * =============
   *
   * for flat top response particles the mean energy to produce
   * a photon is 19.5 eV => Y = 1/19.5 = 0.051
   *
   * At zero electric field, for not-flat-top particles, the scintillation yield,
   * relative to the one of flat top particles is:
   *  Y_e = 0.8 Y
   *  Y_alpha = 0.7 Y
   *  Y_recoils = 0.2-0.4
   *
   * References
   * ---------
   *
   * [1] http://iopscience.iop.org/article/10.1143/JJAP.41.1538/pdf
   * [2] https://www.mpi-hd.mpg.de/gerda/public/2016/phd2016_bjoernLehnert.pdf
   * [3] "Attenuation of the scintillation light in liquid argon in the GERDA Experiment"
   */

  RMGLog::Out(RMGLog::detail, "Using LAr flat-top photon yield of ",
      fLArProperties.flat_top_photon_yield/CLHEP::keV, " / keV");

  lar_mpt->AddConstProperty("SCINTILLATIONYIELD", fLArProperties.flat_top_photon_yield);

  /* Yield ratio
   *
   * This is the value for electrons and gammas
   * For example, for nuclear recoils it should be 0.75
   * nominal value for electrons and gammas: 0.23 (WArP data)
   */

  lar_mpt->AddConstProperty("YIELDRATIO", 0.23);

  /* Singlet and triplet lifetimes
   *
   * Can be set via macro commands:
   *   /RMG/Materials/LAr/SingletLifetime <value> <unit>
   *   /RMG/Materials/LAr/TripletLifetime <value> <unit>
   * default values of 5.95ns and 1um can be found in the messenger class
   *
   * The triplet lifetime is the one measured during GERDA PhaseII
   * Seems to be quite constant over time for the whole length of PhaseII
   * https://www.mpi-hd.mpg.de/gerda/internal/Catania18/slides/20181112_DataSelectionLArBI.pdf
   */

  RMGLog::Out(RMGLog::detail, "Using LAr singlet lifetime of ", fLArProperties.singlet_lifetime/CLHEP::ns," ns");
  RMGLog::Out(RMGLog::detail, "Using LAr triples lifetime of ", fLArProperties.singlet_lifetime/CLHEP::us," us");

  lar_mpt->AddConstProperty("FASTTIMECONSTANT", fLArProperties.singlet_lifetime);
  lar_mpt->AddConstProperty("SLOWTIMECONSTANT", fLArProperties.triplet_lifetime);

  /* Attenuation length
   *
   * The Dresden group measured an absorption length of 15cm at 128nm,
   * see "Attenuation of the scintillation light in liquid argon in the GERDA Experiment"
   * (paper still in prep at the time of writing)
   *
   * We don't know how the attenuation length actually varies with the wavelength, so here
   * we use a custom exponential function just to avoid a step-like function. Still is a
   * guess, but it's possible to rescale the curve via macro command:
   *   /RMG/Materials/LAr/AbsorptionLength <value> <unit>
   */

  auto lar_absorption_length = [](double lambda) {
    auto l = 5.976e-12*CLHEP::cm * std::exp((0.223/CLHEP::nm) * lambda);
    return l > 1000*CLHEP::m ? 1000*CLHEP::m : l; // avoid large numbers
  };

  RMGLog::Out(RMGLog::detail, "LAr absorption length at 128nm set to ",
      fLArProperties.vuv_absorption_length/CLHEP::cm, " cm");

  E1 = to_energy(650*CLHEP::nm);
  E2 = to_energy(113*CLHEP::nm);
  dE = (E2 - E1)/100;
  for (int e = E1; e <= E2; e += dE) {
    lar_mpt->AddEntry("RINDEX",    e, std::sqrt(lar_dielectric_const(to_wavelength(e))));
    lar_mpt->AddEntry("RAYLEIGH",  e, lar_rayleigh_length(to_wavelength(e),
          RMGMaterialTable::GetMaterial("LiquidArgon")->GetTemperature()));

    // normalize absorption length
    lar_mpt->AddEntry("ABSLENGTH", e, lar_absorption_length(to_wavelength(e))
        * fLArProperties.vuv_absorption_length / lar_absorption_length(128*CLHEP::nm));
  }

  RMGMaterialTable::GetMaterial("LiquidArgon")->SetMaterialPropertiesTable(lar_mpt);

  /* LAr Fano factor
   * statistical yield fluctuation can be broadened or narrower
   * (impurities, fano factor):
   *
   * LAr Fano factor = 0.11 ( Doke et al, NIM 134 (1976)353 )
   */

  lar_mpt->AddConstProperty("RESOLUTIONSCALE", 0.11);

  /* Birk's constant
   *
   */

  RMGMaterialTable::GetMaterial("LiquidArgon")->GetIonisation()->SetBirksConstant(5.1748e-4*CLHEP::cm/CLHEP::MeV);
}

void RMGMaterialTable::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Materials",
      "Commands for controlling material definitions");

  fLArMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Materials/LAr",
      "Commands for controlling LAr specifications");

  new G4UnitDefinition("1/eV", "1/eV", "ScintillationYield", 1./CLHEP::eV);
  new G4UnitDefinition("1/keV", "1/keV", "ScintillationYield", 1./CLHEP::keV);
  new G4UnitDefinition("1/MeV", "1/MeV", "ScintillationYield", 1./CLHEP::MeV);

  fLArMessenger->DeclarePropertyWithUnit("FlatTopPhotonYield", "1/keV", fLArProperties.flat_top_photon_yield)
    .SetGuidance("LAr photon yield for flat-top particles")
    .SetUnitCategory("ScintillationYield")
    .SetParameterName("Y", false)
    .SetRange("Y > 0")
    .SetToBeBroadcasted(false)
    .SetStates(G4State_PreInit, G4State_Init);

  fLArMessenger->DeclarePropertyWithUnit("SingletLifetime", "ns", fLArProperties.singlet_lifetime)
    .SetGuidance("Lifetime of the LAr singlet state")
    .SetUnitCategory("Time")
    .SetParameterName("T", false)
    .SetRange("T > 0")
    .SetToBeBroadcasted(false)
    .SetStates(G4State_PreInit, G4State_Init);

  fLArMessenger->DeclarePropertyWithUnit("TripletLifetime", "ns", fLArProperties.triplet_lifetime)
    .SetGuidance("Lifetime of the LAr triplet state")
    .SetUnitCategory("Time")
    .SetParameterName("T", false)
    .SetRange("T > 0")
    .SetToBeBroadcasted(false)
    .SetStates(G4State_PreInit, G4State_Init);

  fLArMessenger->DeclarePropertyWithUnit("VUVAbsorptionLength", "cm", fLArProperties.vuv_absorption_length)
    .SetGuidance("LAr absorption length at 128 nm wavelength")
    .SetUnitCategory("Length")
    .SetParameterName("L", false)
    .SetRange("L > 0")
    .SetToBeBroadcasted(false)
    .SetStates(G4State_PreInit, G4State_Init);
}

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMG_MATERIAL_TABLE_HH_
#define _RMG_MATERIAL_TABLE_HH_

#include <memory>
#include <map>

#include "globals.hh"
#include "G4GenericMessenger.hh"

class G4Material;
class RMGMaterialTable {

  public:

    enum BathMaterial {
      kLiquidArgon,
      kLiquidNitrogen,
      kLiquidXenon,
      kVacuum,
      kNone
    };

    RMGMaterialTable();
    ~RMGMaterialTable() = default;

    RMGMaterialTable           (RMGMaterialTable const&) = delete;
    RMGMaterialTable& operator=(RMGMaterialTable const&) = delete;
    RMGMaterialTable           (RMGMaterialTable&&)      = delete;
    RMGMaterialTable& operator=(RMGMaterialTable&&)      = delete;

    static G4Material* GetMaterial(G4String);
    static G4Material* GetMaterial(BathMaterial);

    struct LArProperties {
      G4double flat_top_photon_yield;
      G4double singlet_lifetime;
      G4double triplet_lifetime;
      G4double vuv_absorption_length;
    };

    struct PropertiesAtTemperature {
      PropertiesAtTemperature() = default;
      inline PropertiesAtTemperature(G4String name, G4double ge_dens, G4double enr_ge_dens) :
        g4_name(name), germanium_density(ge_dens), enriched_germanium_density(enr_ge_dens) {}

      G4String g4_name;
      G4double germanium_density;
      G4double enriched_germanium_density;
    };

  private:

    void InitializeMaterials();
    void InitializeOpticalProperties();
    void InitializeLArOpticalProperties();

    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<G4GenericMessenger> fLArMessenger;
    void DefineCommands();

    static std::map<G4String, G4String> fMaterialAliases;
    LArProperties fLArProperties;
    static std::map<BathMaterial, PropertiesAtTemperature> fPropertiesAtTemperatureTable;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

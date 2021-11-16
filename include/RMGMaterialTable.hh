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
      kAir
    };

    RMGMaterialTable();
    ~RMGMaterialTable() = default;

    RMGMaterialTable           (RMGMaterialTable const&) = delete;
    RMGMaterialTable& operator=(RMGMaterialTable const&) = delete;
    RMGMaterialTable           (RMGMaterialTable&&)      = delete;
    RMGMaterialTable& operator=(RMGMaterialTable&&)      = delete;

    static G4Material* GetMaterial(std::string);
    static G4Material* GetMaterial(BathMaterial);

    struct LArProperties {
      double flat_top_photon_yield;
      double singlet_lifetime;
      double triplet_lifetime;
      double vuv_absorption_length;
    };

    struct PropertiesAtTemperature {
      PropertiesAtTemperature() = default;
      inline PropertiesAtTemperature(std::string name, double ge_dens, double enr_ge_dens) :
        g4_name(name), germanium_density(ge_dens), enriched_germanium_density(enr_ge_dens) {}

      std::string g4_name;
      double germanium_density;
      double enriched_germanium_density;
    };

  private:

    void InitializeMaterials();
    void InitializeOpticalProperties();
    void InitializeLArOpticalProperties();

    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<G4GenericMessenger> fLArMessenger;
    void DefineCommands();

    static std::map<std::string, std::string> fMaterialAliases;
    LArProperties fLArProperties;
    static std::map<BathMaterial, PropertiesAtTemperature> fPropertiesAtTemperatureTable;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMG_MATERIAL_TABLE_HH_
#define _RMG_MATERIAL_TABLE_HH_

#include <memory>
#include <map>

#include "globals.hh"

class G4Material;
class RMGMaterialTableMessenger;
class RMGMaterialTable {

  public:

    enum BathMaterial {
      kLiquidArgon,
      kLiquidNitrogen,
      kLiquidXenon,
      kVacuum,
      kUndefined
    };

    RMGMaterialTable();
    ~RMGMaterialTable();

    RMGMaterialTable           (RMGMaterialTable const&) = delete;
    RMGMaterialTable& operator=(RMGMaterialTable const&) = delete;
    RMGMaterialTable           (RMGMaterialTable&&)      = delete;
    RMGMaterialTable& operator=(RMGMaterialTable&&)      = delete;

    static G4Material* GetMaterial(G4String);
    static G4Material* GetMaterial(BathMaterial);

    inline void SetLArFlatTopPhotonYield(G4double y) { fLArProperties.flat_top_photon_yield = y; }
    inline void SetLArSingletLifetime(G4double t) { fLArProperties.singlet_lifetime = t; }
    inline void SetLArTripletLifetime(G4double t) { fLArProperties.triplet_lifetime = t; }
    inline void SetLArVUVAbsorptionLength(G4double l) { fLArProperties.vuv_absorption_length = l; }

  private:

    void InitializeMaterials();
    void InitializeOpticalProperties();
    void InitializeLArOpticalProperties();

    static std::map<G4String, G4String> fMaterialAliases;
    std::unique_ptr<RMGMaterialTableMessenger> fG4Messenger;

    struct LArProperties {
      G4double flat_top_photon_yield;
      G4double singlet_lifetime;
      G4double triplet_lifetime;
      G4double vuv_absorption_length;
    };

    LArProperties fLArProperties;

    struct PropertiesAtTemperature {
      PropertiesAtTemperature() = default;
      inline PropertiesAtTemperature(G4String name, G4double ge_dens, G4double enr_ge_dens) :
        g4_name(name), germanium_density(ge_dens), enriched_germanium_density(enr_ge_dens) {}

      G4String g4_name;
      G4double germanium_density;
      G4double enriched_germanium_density;
    };

    static std::map<BathMaterial, PropertiesAtTemperature> fPropertiesAtTemperatureTable;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

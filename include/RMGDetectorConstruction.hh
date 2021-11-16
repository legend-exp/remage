#ifndef _RMG_MANAGEMENT_DETECTOR_CONSTRUCTION_HH_
#define _RMG_MANAGEMENT_DETECTOR_CONSTRUCTION_HH_

#include <map>
#include <memory>
#include <vector>

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"

#include "RMGMaterialTable.hh"
#include "RMGNavigationTools.hh"

class G4VPhysicalVolume;
class G4GenericMessenger;
class G4VPhysicalVolume;
class RMGDetectorConstruction : public G4VUserDetectorConstruction {

  public:

    RMGDetectorConstruction();
    ~RMGDetectorConstruction() = default;

    RMGDetectorConstruction           (RMGDetectorConstruction const&) = delete;
    RMGDetectorConstruction& operator=(RMGDetectorConstruction const&) = delete;
    RMGDetectorConstruction           (RMGDetectorConstruction&&)      = delete;
    RMGDetectorConstruction& operator=(RMGDetectorConstruction&&)      = delete;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    inline void IncludeGDMLFile(std::string filename) { fGDMLFiles.emplace_back(filename); }
    inline virtual G4VPhysicalVolume* DefineGeometry() { return nullptr; }
    inline void SetMaxStepLimit(std::string name, double max_step) {
      fPhysVolStepLimits.insert_or_assign(name, max_step);
    }
    static inline RMGMaterialTable::BathMaterial GetBathMaterial() { return fBathMaterial; }
    inline void PrintListOfLogicalVolumes() { RMGNavigationTools::PrintListOfLogicalVolumes(); }
    inline void PrintListOfPhysicalVolumes() { RMGNavigationTools::PrintListOfPhysicalVolumes(); }

  private:

    std::vector<std::string> fGDMLFiles;
    std::unique_ptr<RMGMaterialTable> fMaterialTable;
    std::map<std::string, double> fPhysVolStepLimits;
    static RMGMaterialTable::BathMaterial fBathMaterial;
    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    G4VPhysicalVolume* fWorld;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

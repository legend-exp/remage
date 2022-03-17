#ifndef _RMG_MANAGEMENT_DETECTOR_CONSTRUCTION_HH_
#define _RMG_MANAGEMENT_DETECTOR_CONSTRUCTION_HH_

#include <string>
#include <set>
#include <map>
#include <memory>
#include <vector>

#include "G4VUserDetectorConstruction.hh"
#include "G4Region.hh"

#include "RMGMaterialTable.hh"
#include "RMGNavigationTools.hh"

class G4GenericMessenger;
class G4VPhysicalVolume;
class RMGHardware : public G4VUserDetectorConstruction {

  public:

    RMGHardware();
    ~RMGHardware() = default;

    RMGHardware           (RMGHardware const&) = delete;
    RMGHardware& operator=(RMGHardware const&) = delete;
    RMGHardware           (RMGHardware&&)      = delete;
    RMGHardware& operator=(RMGHardware&&)      = delete;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    enum DetectorType {
      kGermanium,
      kOptical,
      kLAr
    };

    struct DetectorMetadata {
      DetectorType type;
      int uid;
    };

    void RegisterDetector(DetectorType type, const std::string& pv_name, int uid, int copy_nr=0);
    inline const auto& GetDetectorMetadataMap() { return fDetectorMetadata; }
    inline const auto& GetDetectorMetadata(std::pair<std::string, int> det) {
      return fDetectorMetadata.at(det);
    }
    inline const auto& GetActiveDetectorList() { return fActiveDetectors; }

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
    std::unique_ptr<RMGMaterialTable> fMaterialTable = nullptr;
    std::map<std::string, double> fPhysVolStepLimits;
    static RMGMaterialTable::BathMaterial fBathMaterial;

    // one for each physical volume
    std::map<std::pair<std::string, int>, DetectorMetadata> fDetectorMetadata;
    std::set<DetectorType> fActiveDetectors;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    G4VPhysicalVolume* fWorld = nullptr;
    // G4Region* fSensitiveRegion = new G4Region("SensitiveRegion");
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

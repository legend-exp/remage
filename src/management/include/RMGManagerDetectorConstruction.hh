#ifndef _RMG_MANAGEMENT_DETECTOR_CONSTRUCTION_HH_
#define _RMG_MANAGEMENT_DETECTOR_CONSTRUCTION_HH_

#include <map>
#include <memory>

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "RMGMaterialTable.hh"

class G4VPhysicalVolume;
class RMGManagerDetectorConstruction : public G4VUserDetectorConstruction {

  public:

    RMGManagerDetectorConstruction();
    ~RMGManagerDetectorConstruction() = default;

    RMGManagerDetectorConstruction           (RMGManagerDetectorConstruction const&) = delete;
    RMGManagerDetectorConstruction& operator=(RMGManagerDetectorConstruction const&) = delete;
    RMGManagerDetectorConstruction           (RMGManagerDetectorConstruction&&)      = delete;
    RMGManagerDetectorConstruction& operator=(RMGManagerDetectorConstruction&&)      = delete;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    virtual void DefineGeometry() = 0;
    inline void SetMaxStepLimit(G4String name, double max_step) { fPhysVolStepLimits.at(name) = max_step; }
    static inline RMGMaterialTable::BathMaterial GetBathMaterial() { return fBathMaterial; }

  private:

    std::unique_ptr<RMGMaterialTable> fMaterialTable;
    std::map<G4String, G4double> fPhysVolStepLimits;
    static RMGMaterialTable::BathMaterial fBathMaterial;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

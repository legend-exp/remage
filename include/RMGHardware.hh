// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _RMG_HARDWARE_HH_
#define _RMG_HARDWARE_HH_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "G4GenericMessenger.hh"
#include "G4Region.hh"
#include "G4VUserDetectorConstruction.hh"

#include "RMGHardwareMessenger.hh"
#include "RMGNavigationTools.hh"
#include "RMGVOutputScheme.hh"

class G4VPhysicalVolume;
class RMGHardware : public G4VUserDetectorConstruction {

  public:

    RMGHardware();
    ~RMGHardware() = default;

    RMGHardware(RMGHardware const&) = delete;
    RMGHardware& operator=(RMGHardware const&) = delete;
    RMGHardware(RMGHardware&&) = delete;
    RMGHardware& operator=(RMGHardware&&) = delete;

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

    void RegisterDetector(DetectorType type, const std::string& pv_name, int uid, int copy_nr = 0,
        bool allow_uid_reuse = false);
    inline const auto& GetDetectorMetadataMap() { return fDetectorMetadata; }
    inline const auto& GetDetectorMetadata(const std::pair<std::string, int>& det) {
      return fDetectorMetadata.at(det);
    }
    inline const auto& GetActiveDetectorList() { return fActiveDetectors; }
    [[nodiscard]] inline const auto& GetAllActiveOutputSchemes() { return fActiveOutputSchemes; }

    inline void IncludeGDMLFile(std::string filename) { fGDMLFiles.emplace_back(filename); }
    inline virtual G4VPhysicalVolume* DefineGeometry() { return nullptr; }
    inline void SetMaxStepLimit(std::string name, double max_step) {
      fPhysVolStepLimits.insert_or_assign(name, max_step);
    }
    inline void PrintListOfLogicalVolumes() { RMGNavigationTools::PrintListOfLogicalVolumes(); }
    inline void PrintListOfPhysicalVolumes() { RMGNavigationTools::PrintListOfPhysicalVolumes(); }

  private:

    /// List of GDML files to load
    std::vector<std::string> fGDMLFiles;
    bool fGDMLDisableOverlapCheck = false;
    int fGDMLOverlapCheckNumPoints = 3000;
    /// Mapping between physical volume names and maximum (user) step size to apply
    std::map<std::string, double> fPhysVolStepLimits;

    // one element for each sensitive detector physical volume
    std::map<std::pair<std::string, int>, DetectorMetadata> fDetectorMetadata;
    std::set<DetectorType> fActiveDetectors;
    static G4ThreadLocal std::vector<std::shared_ptr<RMGVOutputScheme>> fActiveOutputSchemes;
    static G4ThreadLocal bool fActiveDetectorsInitialized;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<RMGHardwareMessenger> fHwMessenger;
    void DefineCommands();

    /// The world volume
    G4VPhysicalVolume* fWorld = nullptr;

    /** Region used to assign special production cuts
     *  for sensitive volumes. Logical volumes of sensitive volumes should be
     *  added to it. */
    G4Region* fSensitiveRegion = new G4Region("SensitiveRegion");
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

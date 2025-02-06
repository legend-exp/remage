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

#include "RMGDetectorMetadata.hh"
#include "RMGHardwareMessenger.hh"
#include "RMGNavigationTools.hh"
#include "RMGVOutputScheme.hh"

/** @brief Class to handle the detector geometry hardware, extends @c G4VUserDetectorConstruction . */
class G4VPhysicalVolume;
class RMGHardware : public G4VUserDetectorConstruction {

  public:

    /** @brief Constructor based on calling the macro commands (with @c DefineCommands ). */
    RMGHardware();
    ~RMGHardware() = default;

    RMGHardware(RMGHardware const&) = delete;
    RMGHardware& operator=(RMGHardware const&) = delete;
    RMGHardware(RMGHardware&&) = delete;
    RMGHardware& operator=(RMGHardware&&) = delete;

    /** @brief Construct the detector 
     * 
     *  @details All detector geometry must be based on GDML files, parsed with @c G4GDMLParser .
     *  This method defines the geoemetry and checks for overlaps (if not disabled). It also
     *  assigns physical volumes to Geant4 regions and sets user step limits.
     *
     * @returns The physical volume of the world.
     * 
     */
    G4VPhysicalVolume* Construct() override;

    /** @brief Method to set the sensitive detectors. 
     * 
     * @details Setup thread-local data, called once for the master thread and once for each worker thread.
     */
    void ConstructSDandField() override;

    /** @brief Register a detector as being a remage sensitive detector.
      * 
      * @param type The type of detector.
      * @param pv_name The name of the physical volume.
      * @param uid A unique integer identifier for the sensitive volume.
      * @param copy_nr The copy number for the physical volume.
      * @param allow_uid_reuse Flag to allow a uid to be reused
      */
    void RegisterDetector(RMGDetectorType type, const std::string& pv_name, int uid,
        int copy_nr = 0, bool allow_uid_reuse = false);

    /** @brief Extract a map of the detector metadata, one element for each sensitive detector physical volume and copy_nr. */
    inline const auto& GetDetectorMetadataMap() { return fDetectorMetadata; }

    /** @brief Extract the detector metdata for a given detector.
      * 
      * @param det the detector identifier, a pair of the physical volume name and the copy number.
      */
    inline const auto& GetDetectorMetadata(const std::pair<std::string, int>& det) {
      return fDetectorMetadata.at(det);
    }

    
    inline const auto& GetActiveDetectorList() { return fActiveDetectors; }
    [[nodiscard]] inline const auto& GetAllActiveOutputSchemes() { return fActiveOutputSchemes; }

    /** @brief Add a GDML file to the geometry.
      *
      * @param filename file name to add. 
      */
    inline void IncludeGDMLFile(std::string filename) { fGDMLFiles.emplace_back(filename); }

    /** @brief Method to define geometry directly, the user must reimplement the base class method. */
    inline virtual G4VPhysicalVolume* DefineGeometry() { return nullptr; }

    /** @brief Set the maximum step size for a certain physical volume.
      * 
      * @details This is used as a @c G4UserLimit to limit step sizes to being no larger than the chosen value.
      * This requires the @c G4StepLimiter process to be activated in the physics list.
      * 
      * @param name The name of the physical volume to add step limits to.
      * @param max_step The maximum step size.
      */
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
    std::map<std::pair<std::string, int>, RMGDetectorMetadata> fDetectorMetadata;
    std::set<RMGDetectorType> fActiveDetectors;
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

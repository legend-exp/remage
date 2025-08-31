// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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
#include "RMGLog.hh"
#include "RMGNavigationTools.hh"
#include "RMGVOutputScheme.hh"

/** @brief Class to handle the detector geometry hardware, extends @c G4VUserDetectorConstruction. */
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
     *  @details Detector geometry can be based on GDML files, parsed with @c G4GDMLParser .
     *  Alternatively geometry can be defined directly by overriding the @c DefineGeometry() method.
     *
     *  This function defines the geometry and checks for overlaps, if using GDML defined geometry
     * and check are not disabled. It also assigns physical volumes to Geant4 regions and sets user
     * step limits. This must not modify thread-local state, because it is only called once
     * globally.
     *
     * This function will call @c RegisterDetector() to register detectors staged with @c
     * StageDetector() and register all detector types specified with the
     * `RegisterDetectorsFromGDML` macro command from the GDML.
     *
     * @returns The physical volume of the world.
     */
    G4VPhysicalVolume* Construct() override;

    /** @brief Setup thread-local geometry data.
     *
     * @details This is called once for the master thread and once for each worker thread. Can be
     * used to setup sensitive detectors, which have to be constructed per thread.
     */
    void ConstructSDandField() override;

    /** @brief Register a physical volume as sensitive detector.
     *
     * @details The @c uid is a unique identifier for the detector. It is
     * mostly used to label the detector in the simulation output. This
     * function also informs the run action to automatically activate output
     * schemes for all registered detector types.
     * This function is called during @c Construct() method to register detectors
     * from the GDML and to register detectors staged with @c StageDetector() method.
     *
     * @param type The type of detector.
     * @param pv_name The name of the physical volume to be registered.
     * @param uid A unique integer identifier for the sensitive volume.
     * @param copy_nr The copy number of the physical volume.
     * @param allow_uid_reuse Flag to allow assigning the same @c uid to different detectors.
     */
    void RegisterDetector(
        RMGDetectorType type,
        const std::string& pv_name,
        int uid,
        int copy_nr = 0,
        bool allow_uid_reuse = false
    );

    /** @brief Stage a detector for later registration.
     *
     * @details This function is used to stage detectors following a regex name pattern, which will
     * be registered later during the @c Construct() method. This function will be called by the @c
     * /RegisterDetector macro command instead of the @c RegisterDetector() function directly. If
     * multiple volumes match a regex by given uid, depending on the @c allow_uid_reuse flag, the
     * uid will be reused or incremented for each detector.
     *
     * @param type The type of detector.
     * @param pv_name The name of the physical volume to be registered.
     * @param uid A unique integer identifier for the sensitive volume.
     * @param copy_nr The copy number of the physical volume.
     * @param allow_uid_reuse Flag to allow assigning the same @c uid to different detectors.
     */
    void StageDetector(
        RMGDetectorType type,
        const std::string& pv_name,
        int uid,
        const std::string& copy_nr = "0",
        bool allow_uid_reuse = false
    );

    /** @brief Extract a map of the detector metadata, one element for each sensitive detector physical volume and copy_nr. */
    const auto& GetDetectorMetadataMap() { return fDetectorMetadata; }

    /** @brief Extract the detector metadata for a given detector.
     *
     * @param det the detector identifier, a pair of the physical volume name and the copy number.
     */
    const auto& GetDetectorMetadata(const std::pair<std::string, int>& det) {
      return fDetectorMetadata.at(det);
    }


    const auto& GetActiveDetectorList() { return fActiveDetectors; }
    [[nodiscard]] const auto& GetAllActiveOutputSchemes() { return fActiveOutputSchemes; }

    /** @brief Add a GDML file to the geometry.
     *
     * @param filename file name to add.
     */
    void IncludeGDMLFile(std::string filename) { fGDMLFiles.emplace_back(filename); }

    /** @brief Method to define geometry directly, the user must reimplement the base class method. */
    virtual G4VPhysicalVolume* DefineGeometry() { return nullptr; }

    /** @brief Set the maximum step size.
     *
     * @details This is used as a @c G4UserLimit to limit step sizes to being no larger than the
     * chosen value. This requires the @c G4StepLimiter process to be activated in the physics list.
     *
     * @param max_step The maximum step size.
     * @param name Name of the physical volume.
     */
    void SetMaxStepLimit(double max_step, std::string name);

    void PrintListOfLogicalVolumes() { RMGNavigationTools::PrintListOfLogicalVolumes(); }
    void PrintListOfPhysicalVolumes() { RMGNavigationTools::PrintListOfPhysicalVolumes(); }

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

    std::set<RMGDetectorType> fRegisterDetectorsFromGDML;
    void RegisterDetectorsFromGDML(std::string s);

    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<RMGHardwareMessenger> fHwMessenger;
    void DefineCommands();

    /// The world volume
    G4VPhysicalVolume* fWorld = nullptr;

    /** Region used to assign special production cuts
     *  for sensitive volumes. Logical volumes of sensitive volumes should be
     *  added to it. */
    G4Region* fSensitiveRegion = new G4Region("SensitiveRegion");

    struct RMGStagedDetector {
        RMGDetectorType type;
        std::string name;
        int uid;
        std::string copy_nr;
        bool allow_uid_reuse;
    };

    // Holds detector info before initialization
    std::map<std::pair<std::string, std::string>, RMGStagedDetector> fStagedDetectors;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

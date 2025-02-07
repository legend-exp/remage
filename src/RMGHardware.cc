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

#include "RMGHardware.hh"

#include <filesystem>
namespace fs = std::filesystem;

#include "G4GenericMessenger.hh"
#include "G4GeomTestVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4UserLimits.hh"
#include "G4VPhysicalVolume.hh"

#include "RMGConfig.hh"
#include "RMGGermaniumDetector.hh"
#include "RMGGermaniumOutputScheme.hh"
#include "RMGHardwareMessenger.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGNavigationTools.hh"
#include "RMGOpticalDetector.hh"
#include "RMGOpticalOutputScheme.hh"
#include "RMGScintillatorDetector.hh"
#include "RMGScintillatorOutputScheme.hh"
#include "RMGVertexOutputScheme.hh"

#if RMG_HAS_GDML
#include "G4GDMLParser.hh"
#endif

#include "magic_enum/magic_enum.hpp"

G4ThreadLocal std::vector<std::shared_ptr<RMGVOutputScheme>> RMGHardware::fActiveOutputSchemes = {};

G4ThreadLocal bool RMGHardware::fActiveDetectorsInitialized = false;

RMGHardware::RMGHardware() { this->DefineCommands(); }

G4VPhysicalVolume* RMGHardware::Construct() {

  RMGLog::Out(RMGLog::debug, "Constructing detector");

  if (!fGDMLFiles.empty()) {
#if RMG_HAS_GDML
    RMGLog::Out(RMGLog::debug, "Setting up G4GDMLParser");
    G4GDMLParser parser;
    parser.SetOverlapCheck(false); // overlap check is performed below.
    for (const auto& file : fGDMLFiles) {
      RMGLog::Out(RMGLog::detail, "Reading ", file, " GDML file");
      if (!fs::exists(fs::path(file.data()))) RMGLog::Out(RMGLog::fatal, file, " does not exist");
      // TODO: decide here
      parser.Read(file, false);
    }
    fWorld = parser.GetWorldVolume();

    // Check for overlaps, but with no verbose output.
    if (!fGDMLDisableOverlapCheck) {
      RMGLog::Out(RMGLog::summary, "Checking for overlaps in GDML geometry...");
      auto test_vol =
          new G4GeomTestVolume(fWorld, 0, fGDMLOverlapCheckNumPoints, /* verbosity = */ false);
      test_vol->TestOverlapInTree();
    }
#else
    RMGLog::OutDev(RMGLog::fatal, "GDML support is not available!");
#endif
  } else {
    fWorld = this->DefineGeometry();
    if (!fWorld)
      RMGLog::Out(RMGLog::fatal, "DefineGeometry() returned nullptr. ",
          "Did you forget to reimplement the base class method, or to specify a GDML file?");
  }

  // attach user max step sizes to logical volumes
  for (const auto& el : fPhysVolStepLimits) {
    RMGLog::OutFormat(RMGLog::debug, "Setting max user step size for volume '{}' to {}", el.first,
        el.second);
    auto vol = RMGNavigationTools::FindPhysicalVolume(el.first);
    if (!vol) {
      RMGLog::Out(RMGLog::error, "Returned volume is null, skipping user step limit setting");
    } else vol->GetLogicalVolume()->SetUserLimits(new G4UserLimits(el.second));
  }

  for (const auto& [k, v] : fDetectorMetadata) {
    const auto& pv = RMGNavigationTools::FindPhysicalVolume(k.first, k.second);
    if (!pv) RMGLog::Out(RMGLog::fatal, "Could not find detector physical volume");
    const auto lv = pv->GetLogicalVolume();
    // only set to sensitive region if not already done
    if (lv->GetRegion()) {
      RMGLog::OutFormatDev(RMGLog::debug, "Logical volume {} is already assigned to region {}",
          lv->GetName(), lv->GetRegion()->GetName());
    } else {
      RMGLog::OutFormat(RMGLog::debug, "Assigning logical volume {} to region {}", lv->GetName(),
          fSensitiveRegion->GetName());
      lv->SetRegion(fSensitiveRegion);
      fSensitiveRegion->AddRootLogicalVolume(lv);
    }
  }

  return fWorld;
}

void RMGHardware::ConstructSDandField() {
  // setup thread-local data, called once for the master thread and once for each worker thread.

  // set up G4SDManager
  auto sd_man = G4SDManager::GetSDMpointer();
  if (RMGLog::GetLogLevel() <= RMGLog::debug) sd_man->SetVerboseLevel(1);
  else sd_man->SetVerboseLevel(0);

  // map holding a list of sensitive detectors to activate
  std::map<RMGDetectorType, G4VSensitiveDetector*> active_dets;

  for (const auto& [k, v] : fDetectorMetadata) {

    // initialize a concrete detector, if not done yet
    // TODO: allow user to register custom detectors
    if (active_dets.find(v.type) == active_dets.end()) {
      RMGLog::Out(RMGLog::debug, "Registering new sensitive detector of type ",
          magic_enum::enum_name(v.type));

      G4VSensitiveDetector* obj = nullptr;
      std::shared_ptr<RMGVOutputScheme> output;
      switch (v.type) {
        case RMGDetectorType::kOptical:
          obj = new RMGOpticalDetector();
          output = std::make_shared<RMGOpticalOutputScheme>();
          break;
        case RMGDetectorType::kGermanium:
          obj = new RMGGermaniumDetector();
          output = std::make_shared<RMGGermaniumOutputScheme>();
          break;
        case RMGDetectorType::kScintillator:
          obj = new RMGScintillatorDetector();
          output = std::make_shared<RMGScintillatorOutputScheme>();
          break;
        default:
          RMGLog::OutDev(RMGLog::fatal, "No behaviour for sensitive detector type '",
              magic_enum::enum_name<RMGDetectorType>(v.type), "' implemented (implement me)");
      }
      sd_man->AddNewDetector(obj);
      active_dets.emplace(v.type, obj);

      if (!output) {
        RMGLog::OutDev(RMGLog::fatal, "No output scheme sensitive detector type '",
            magic_enum::enum_name(v.type), "' implemented (implement me)");
      }
      fActiveOutputSchemes.emplace_back(output);
    }

    // now assign logical volumes to the sensitive detector
    const auto& pv = RMGNavigationTools::FindPhysicalVolume(k.first, k.second);
    if (!pv) RMGLog::Out(RMGLog::fatal, "Could not find detector physical volume");
    const auto lv = pv->GetLogicalVolume();
    // only add the SD to the LV if not already present.
    if (lv->GetSensitiveDetector() != active_dets[v.type]) {
      this->SetSensitiveDetector(lv, active_dets[v.type]);
    }

    RMGLog::OutFormat(RMGLog::debug,
        "Registered new sensitive detector volume of type {}: {} (uid={}, lv={})",
        magic_enum::enum_name(v.type), pv->GetName().c_str(), v.uid, lv->GetName().c_str());
  }

  // also store primary vertex data, if we have any other output.
  if (!fActiveOutputSchemes.empty()) {
    fActiveOutputSchemes.emplace_back(std::make_shared<RMGVertexOutputScheme>());
  }

  // Also add user-provided output schemes.
  for (const auto& os : RMGManager::Instance()->GetUserInit()->GetOutputSchemes()) {
    fActiveOutputSchemes.emplace_back(os());
  }

  std::string vec_repr;
  for (const auto& d : fActiveDetectors) vec_repr += std::string(magic_enum::enum_name(d)) + ", ";
  if (vec_repr.size() > 2) vec_repr.erase(vec_repr.size() - 2);
  RMGLog::OutFormat(RMGLog::debug, "List of activated detectors: [{}]", vec_repr);

  fActiveDetectorsInitialized = true;

  // copy birks constant from material properties, as it cannot be specified in GDML
  for (G4Material* mat : *G4Material::GetMaterialTable()) {
    auto mpt = mat->GetMaterialPropertiesTable();
    if (!mpt) continue;
    if (!mpt->ConstPropertyExists("BIRKSCONSTANT")) continue;

    G4double bc = mpt->GetConstProperty("BIRKSCONSTANT");
    mat->GetIonisation()->SetBirksConstant(bc);
    RMGLog::OutFormat(RMGLog::debug, "Birks constant of material {} set to {} mm/MeV from GDML",
        mat->GetName(), bc / (CLHEP::mm / CLHEP::MeV));
  }
}

void RMGHardware::RegisterDetector(RMGDetectorType type, const std::string& pv_name, int uid,
    int copy_nr, bool allow_uid_reuse) {
  if (fActiveDetectorsInitialized) {
    RMGLog::Out(RMGLog::error,
        "Active detectors cannot be mutated after constructing the detector.");
    return;
  }

  // sanity check for duplicate uids.
  if (!allow_uid_reuse) {
    for (const auto& [k, v] : fDetectorMetadata) {
      if (v.uid == uid) {
        RMGLog::OutFormat(RMGLog::error, "UID {} has already been assigned", uid);
        return;
      }
    }
  }

  // register type of detector to inform the run action
  fActiveDetectors.insert(type);

  // FIXME: can this be done with emplace?
  auto r_value = fDetectorMetadata.insert({{pv_name, copy_nr}, {type, uid, pv_name}});
  if (!r_value.second) { // if insertion did not take place
    RMGLog::OutFormat(RMGLog::warning,
        "Physical volume '{}' (copy number {}) has already been registered as detector", pv_name,
        copy_nr);
  } else {
    RMGLog::OutFormat(RMGLog::detail,
        "Registered physical volume '{}' (copy nr. {}) as {} detector type (uid={})", pv_name,
        copy_nr, magic_enum::enum_name(type), uid);
  }
}

void RMGHardware::SetMaxStepLimit(double max_step) {
  if (this->fVolumeForStepLimit == "")
    RMGLog::OutFormat(RMGLog::error, "cannot set step limits if 'fVolumeForStepLimit' is not set");

  fPhysVolStepLimits.insert_or_assign(this->fVolumeForStepLimit, max_step);

  RMGLog::OutFormat(RMGLog::detail, "Set step limits for {:s} to {:.2f} mm", fVolumeForStepLimit,
      max_step);
}

void RMGHardware::DefineCommands() {

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Geometry/",
      "Commands for controlling geometry definitions"));

  fMessengers.back()
      ->DeclareProperty("GDMLDisableOverlapCheck", fGDMLDisableOverlapCheck)
      .SetGuidance("Disable the automatic overlap check after loading a GDML file")
      .SetStates(G4State_PreInit);

  fMessengers.back()
      ->DeclareProperty("GDMLOverlapCheckNumPoints", fGDMLOverlapCheckNumPoints)
      .SetGuidance("Change the number of points sampled for overlap checks")
      .SetStates(G4State_PreInit);

  fMessengers.back()
      ->DeclareMethod("IncludeGDMLFile", &RMGHardware::IncludeGDMLFile)
      .SetGuidance("Use GDML file for geometry definition")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit);

  fMessengers.back()
      ->DeclareMethod("PrintListOfLogicalVolumes", &RMGHardware::PrintListOfLogicalVolumes)
      .SetGuidance("Print list of defined logical volumes")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethod("PrintListOfPhysicalVolumes", &RMGHardware::PrintListOfPhysicalVolumes)
      .SetGuidance("Print list of defined physical volumes")
      .SetStates(G4State_Idle);

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Geometry/StepLimits/",
      "Commands for setting step limits for volumes"));

  fMessengers.back()
      ->DeclareMethod("AddVolume", &RMGHardware::AddVolumeForStepLimits)
      .SetGuidance("Add physical volume to apply step limits to")
      .SetParameterName("pv_name", false)
      .SetStates(G4State_PreInit);

  fMessengers.back()
      ->DeclareMethodWithUnit("MaxStepSize", "mm", &RMGHardware::SetMaxStepLimit)
      .SetGuidance("Set center position (X coordinate)")
      .SetParameterName("value", false)
      .SetStates(G4State_PreInit);


  // RegisterDetector cannot be defined with the G4GenericMessenger (it has to many parameters).
  fHwMessenger = std::make_unique<RMGHardwareMessenger>(this);
}

// vim: tabstop=2 shiftwidth=2 expandtab

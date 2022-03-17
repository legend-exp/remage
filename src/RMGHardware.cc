#include "RMGHardware.hh"

#include <filesystem>
namespace fs = std::filesystem;

#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UserLimits.hh"
#include "G4GDMLParser.hh"
#include "G4SDManager.hh"

#include "RMGMaterialTable.hh"
#include "RMGLog.hh"
#include "RMGNavigationTools.hh"
#include "RMGOpticalDetector.hh"

#include "magic_enum/magic_enum.hpp"

RMGMaterialTable::BathMaterial RMGHardware::fBathMaterial = RMGMaterialTable::BathMaterial::kAir;

RMGHardware::RMGHardware() {
  fMaterialTable = std::make_unique<RMGMaterialTable>();

  this->DefineCommands();
}

G4VPhysicalVolume* RMGHardware::Construct() {

  RMGLog::Out(RMGLog::debug, "Constructing detector");

  if (!fGDMLFiles.empty()) {
    RMGLog::Out(RMGLog::debug, "Setting up G4GDMLParser");
    G4GDMLParser parser;
    parser.SetOverlapCheck(true);
    for (const auto& file : fGDMLFiles) {
      RMGLog::Out(RMGLog::detail, "Reading ", file, " GDML file");
      if (!fs::exists(fs::path(file.data()))) RMGLog::Out(RMGLog::fatal, file, " does not exist");
      // TODO: decide here
      parser.Read(file, false);
    }
    fWorld = parser.GetWorldVolume();
  }
  else {
    fWorld = this->DefineGeometry();
    if (!fWorld) RMGLog::Out(RMGLog::fatal, "DefineGeometry() returned nullptr. ",
        "Did you forget to reimplement the base class method?");
  }

  // TODO: build and return world volume?

  for (const auto& el : fPhysVolStepLimits) {
    RMGLog::OutFormat(RMGLog::debug, "Setting max user step size for volume '{}' to {}", el.first, el.second);
    auto vol = RMGNavigationTools::FindPhysicalVolume(el.first);
    if (!vol) {
      RMGLog::Out(RMGLog::error, "Returned volume is null, skipping user step limit setting");
    }
    else vol->GetLogicalVolume()->SetUserLimits(new G4UserLimits(el.second));
  }

  // TODO: create sensitive region for special production cuts
  // auto det_lv = RMGNavigationTools::FindLogicalVolume("Detector");
  // det_lv->SetRegion(fSensitiveRegion);
  // fSensitiveRegion->AddRootLogicalVolume(det_lv);

  return fWorld;
}

void RMGHardware::ConstructSDandField() {

  // set up G4SDManager
  auto sd_man = G4SDManager::GetSDMpointer();
  if (RMGLog::GetLogLevelScreen() <= RMGLog::debug) sd_man->SetVerboseLevel(1);
  else sd_man->SetVerboseLevel(0);

  // map holding a list of sensitive detectors to activate
  std::map<DetectorType, G4VSensitiveDetector*> active_dets;

  for (const auto& [k, v] : fDetectorMetadata) {

    // initialize a concrete detector, if not done yet
    // TODO: allow user to register custom detectors
    if (active_dets.find(v.type) == active_dets.end()) {
      RMGLog::Out(RMGLog::debug, "Registering new sensitive detector of type ", magic_enum::enum_name(v.type));

      G4VSensitiveDetector* obj = nullptr;
      switch (v.type) {
        case DetectorType::kOptical :
          obj = new RMGOpticalDetector();
          break;
        case DetectorType::kGermanium :
        case DetectorType::kLAr :
        default : RMGLog::Out(RMGLog::fatal, "No behaviour for sensitive detector type '",
                      magic_enum::enum_name<DetectorType>(v.type), "' implemented (implement me)");
      }
      sd_man->AddNewDetector(obj);
      active_dets.emplace(v.type, obj);
    }

    // now assign logical volumes to the sensitive detector
    // TODO: what does it happen if one adds the same logical volume multiple times?
    const auto& pv = RMGNavigationTools::FindPhysicalVolume(k.first, k.second);
    if (!pv) RMGLog::Out(RMGLog::fatal, "Could not find detector physical volume");
    this->SetSensitiveDetector(pv->GetLogicalVolume(), active_dets[v.type]);
  }

  std::string vec_repr = "";
  for (const auto& d : fActiveDetectors) vec_repr += std::string(magic_enum::enum_name(d)) + ", ";
  if (vec_repr.size() > 2) vec_repr.erase(vec_repr.size() - 2);
  RMGLog::OutFormat(RMGLog::debug, "List of activated detectors: [{}]", vec_repr);
}

void RMGHardware::RegisterDetector(DetectorType type, const std::string& pv_name,
    int uid, int copy_nr) {

  for (const auto& [k, v] : fDetectorMetadata) {
    if (v.uid == uid) RMGLog::Out(RMGLog::error, "UID ", uid, " has already been assigned");
    return;
  }

  // this must be done here to inform the run action in time
  fActiveDetectors.insert(type);

  // FIXME: can this be done with emplace?
  auto r_value = fDetectorMetadata.insert({{pv_name, copy_nr}, {type, uid}});
  if (!r_value.second) RMGLog::OutFormat(RMGLog::warning,
      "Physical volume '{}' (copy number {}) has already been registered as detector",
      pv_name, copy_nr);

  RMGLog::OutFormat(RMGLog::detail, "Registered physical volume '{}' (copy nr. {}) as {} detector type",
      pv_name, copy_nr, magic_enum::enum_name(type));
}

void RMGHardware::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Geometry/",
      "Commands for controlling geometry definitions");

  fMessenger->DeclareMethod("IncludeGDMLFile", &RMGHardware::IncludeGDMLFile)
    .SetGuidance("Use GDML file for geometry definition")
    .SetParameterName("filename", false)
    .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("PrintListOfLogicalVolumes", &RMGHardware::PrintListOfLogicalVolumes)
    .SetGuidance("Print list of defined physical volumes")
    .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("PrintListOfPhysicalVolumes", &RMGHardware::PrintListOfPhysicalVolumes)
    .SetGuidance("Print list of defined physical volumes")
    .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

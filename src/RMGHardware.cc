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

#include "G4GDMLParser.hh"
#include "G4GenericMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4UserLimits.hh"
#include "G4VPhysicalVolume.hh"

#include "RMGGermaniumDetector.hh"
#include "RMGLog.hh"
#include "RMGNavigationTools.hh"
#include "RMGOpticalDetector.hh"

#include "magic_enum/magic_enum.hpp"

RMGHardware::RMGHardware() { this->DefineCommands(); }

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
  } else {
    fWorld = this->DefineGeometry();
    if (!fWorld)
      RMGLog::Out(RMGLog::fatal, "DefineGeometry() returned nullptr. ",
          "Did you forget to reimplement the base class method?");
  }

  // TODO: build and return world volume?

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

  // set up G4SDManager
  auto sd_man = G4SDManager::GetSDMpointer();
  if (RMGLog::GetLogLevel() <= RMGLog::debug) sd_man->SetVerboseLevel(1);
  else sd_man->SetVerboseLevel(0);

  // map holding a list of sensitive detectors to activate
  std::map<DetectorType, G4VSensitiveDetector*> active_dets;

  for (const auto& [k, v] : fDetectorMetadata) {

    // initialize a concrete detector, if not done yet
    // TODO: allow user to register custom detectors
    if (active_dets.find(v.type) == active_dets.end()) {
      RMGLog::Out(RMGLog::debug, "Registering new sensitive detector of type ",
          magic_enum::enum_name(v.type));

      G4VSensitiveDetector* obj = nullptr;
      switch (v.type) {
        case DetectorType::kOptical: obj = new RMGOpticalDetector(); break;
        case DetectorType::kGermanium: obj = new RMGGermaniumDetector(); break;
        case DetectorType::kLAr:
        default:
          RMGLog::OutDev(RMGLog::fatal, "No behaviour for sensitive detector type '",
              magic_enum::enum_name<DetectorType>(v.type), "' implemented (implement me)");
      }
      sd_man->AddNewDetector(obj);
      active_dets.emplace(v.type, obj);
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

  std::string vec_repr = "";
  for (const auto& d : fActiveDetectors) vec_repr += std::string(magic_enum::enum_name(d)) + ", ";
  if (vec_repr.size() > 2) vec_repr.erase(vec_repr.size() - 2);
  RMGLog::OutFormat(RMGLog::debug, "List of activated detectors: [{}]", vec_repr);

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

void RMGHardware::RegisterDetector(DetectorType type, const std::string& pv_name, int uid,
    int copy_nr) {

  // sanity check
  for (const auto& [k, v] : fDetectorMetadata) {
    if (v.uid == uid) {
      RMGLog::OutFormat(RMGLog::error, "UID {} has already been assigned", uid);
      return;
    }
  }

  // register type of detector to inform the run action
  fActiveDetectors.insert(type);

  // FIXME: can this be done with emplace?
  auto r_value = fDetectorMetadata.insert({{pv_name, copy_nr}, {type, uid}});
  if (!r_value.second) { // if insertion did not take place
    RMGLog::OutFormat(RMGLog::warning,
        "Physical volume '{}' (copy number {}) has already been registered as detector", pv_name,
        copy_nr);
  } else {
    RMGLog::OutFormat(RMGLog::detail,
        "Registered physical volume '{}' (copy nr. {}) as {} detector type", pv_name, copy_nr,
        magic_enum::enum_name(type));
  }
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

  // TODO: RegisterDetector() UI command interface
}

// vim: tabstop=2 shiftwidth=2 expandtab

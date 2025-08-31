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
#include "RMGIpc.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGNavigationTools.hh"
#include "RMGOpticalDetector.hh"
#include "RMGOpticalOutputScheme.hh"
#include "RMGScintillatorDetector.hh"
#include "RMGScintillatorOutputScheme.hh"
#include "RMGTools.hh"
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

    // register detectors from the GDML file, as written by pygeomtools.
    // https://legend-pygeom-tools.readthedocs.io/en/stable/metadata.html
    if (!fRegisterDetectorsFromGDML.empty()) {
      const auto aux_list = parser.GetAuxList();
      auto had_mapping = false;
      auto had_detector = false;
      for (const auto& aux : *aux_list) {
        if (aux.type != "RMG_detector" || !aux.auxList) { continue; }
        had_mapping = true;

        auto det_type_str = aux.value;
        det_type_str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(det_type_str[0])));
        const auto det_type = RMGTools::ToEnum<RMGDetectorType>(det_type_str, "detector type");

        if (fRegisterDetectorsFromGDML.find(det_type) == fRegisterDetectorsFromGDML.end()) {
          continue;
        }

        for (const auto& det_aux : *aux.auxList) {
          RegisterDetector(det_type, det_aux.type, std::stoi(det_aux.value));
          had_detector = true;
        }
      }

      if (!had_mapping) {
        RMGLog::Out(RMGLog::fatal, "No detector mapping found in the GDML file");
      }
      if (!had_detector) {
        RMGLog::Out(RMGLog::warning, "Empty detector mapping found in the GDML file");
      }
    }

    // Check for overlaps, but with no verbose output.
    if (!fGDMLDisableOverlapCheck) {
      RMGLog::Out(RMGLog::summary, "Checking for overlaps in GDML geometry...");
      auto test_vol = new G4GeomTestVolume(fWorld, 0, fGDMLOverlapCheckNumPoints, /* verbosity = */ false);
      test_vol->TestOverlapInTree();
    }
#else
    RMGLog::OutDev(RMGLog::fatal, "GDML support is not available!");
#endif
  } else {
    fWorld = this->DefineGeometry();
    if (!fWorld)
      RMGLog::Out(
          RMGLog::fatal,
          "DefineGeometry() returned nullptr. ",
          "Did you forget to reimplement the base class method, or to specify a GDML file?"
      );
  }

  // attach user max step sizes to logical volumes
  for (const auto& el : fPhysVolStepLimits) {
    RMGLog::OutFormat(
        RMGLog::debug,
        "Setting max user step size for volume '{}' to {}",
        el.first,
        el.second
    );
    auto volumes = RMGNavigationTools::FindPhysicalVolume(el.first);
    if (volumes.empty()) {
      RMGLog::Out(
          RMGLog::error,
          "No matching volumes for '{}' found, skipping user step limit setting",
          el.first
      );
    } else {
      for (const auto& vol : volumes) {
        vol->GetLogicalVolume()->SetUserLimits(new G4UserLimits(el.second));
      }
    }
  }

  // register staged detectors now
  if (!fStagedDetectors.empty()) {
    RMGLog::Out(RMGLog::debug, "Registering staged detectors");
    for (const auto& [k, v] : fStagedDetectors) {
      auto volumes = RMGNavigationTools::FindPhysicalVolume(k.first, k.second);

      // Sort alphabetically by name
      std::vector<G4VPhysicalVolume*> sortedVolumes(volumes.begin(), volumes.end());
      // Sorts by name and copy number in ascending order
      std::sort(
          sortedVolumes.begin(),
          sortedVolumes.end(),
          [](G4VPhysicalVolume* a, G4VPhysicalVolume* b) {
            if (a->GetName() == b->GetName()) return a->GetCopyNo() < b->GetCopyNo();
            return a->GetName() < b->GetName();
          }
      );

      int uid = v.uid;

      for (const auto& vol : sortedVolumes) {
        this->RegisterDetector(v.type, vol->GetName(), uid, vol->GetCopyNo(), v.allow_uid_reuse);

        if (!v.allow_uid_reuse) {
          // if we do not allow uid reuse, we give the next detector a new uid
          uid++;
        }
      }
    }
  }

  for (const auto& [k, v] : fDetectorMetadata) {
    auto volumes = RMGNavigationTools::FindPhysicalVolume(k.first, std::to_string(k.second));
    if (volumes.empty()) {
      RMGLog::Out(
          RMGLog::fatal,
          "Could not find detector physical volume for name '{}' (copy number {})",
          k.first,
          k.second
      );
    }
    if (volumes.size() > 1) {
      RMGLog::Out(
          RMGLog::fatal,
          "Found multiple physical volumes for detector Name '{}' (copy number {}) - this is not "
          "allowed",
          k.first,
          k.second
      );
    }
    const auto& pv = *volumes.begin();
    if (!pv) RMGLog::Out(RMGLog::fatal, "Could not find detector physical volume");
    const auto lv = pv->GetLogicalVolume();
    // only set to sensitive region if not already done
    if (lv->GetRegion()) {
      RMGLog::OutFormatDev(
          RMGLog::debug,
          "Logical volume {} is already assigned to region {}",
          lv->GetName(),
          lv->GetRegion()->GetName()
      );
    } else {
      RMGLog::OutFormat(
          RMGLog::debug,
          "Assigning logical volume {} to region {}",
          lv->GetName(),
          fSensitiveRegion->GetName()
      );
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
      RMGLog::Out(
          RMGLog::debug,
          "Registering new sensitive detector of type ",
          magic_enum::enum_name(v.type)
      );

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
          RMGLog::OutDev(
              RMGLog::fatal,
              "No behaviour for sensitive detector type '",
              magic_enum::enum_name<RMGDetectorType>(v.type),
              "' implemented (implement me)"
          );
      }
      sd_man->AddNewDetector(obj);
      active_dets.emplace(v.type, obj);

      if (!output) {
        RMGLog::OutDev(
            RMGLog::fatal,
            "No output scheme sensitive detector type '",
            magic_enum::enum_name(v.type),
            "' implemented (implement me)"
        );
      }
      fActiveOutputSchemes.emplace_back(output);
    }

    // now assign logical volumes to the sensitive detector
    auto volumes = RMGNavigationTools::FindPhysicalVolume(k.first, std::to_string(k.second));
    if (volumes.empty()) {
      RMGLog::Out(
          RMGLog::fatal,
          "Could not find detector physical volume for name '{}' (copy number {})",
          k.first,
          k.second
      );
    }
    if (volumes.size() > 1) {
      RMGLog::Out(
          RMGLog::fatal,
          "Found multiple physical volumes for detector Name '{}' (copy number {}) - this is not "
          "allowed",
          k.first,
          k.second
      );
    }
    const auto& pv = *volumes.begin();
    const auto lv = pv->GetLogicalVolume();
    // only add the SD to the LV if not already present.
    if (lv->GetSensitiveDetector() != active_dets[v.type]) {
      this->SetSensitiveDetector(lv, active_dets[v.type]);
    }

    RMGLog::OutFormat(
        RMGLog::debug,
        "Registered new sensitive detector volume of type {}: {} (uid={}, lv={})",
        magic_enum::enum_name(v.type),
        pv->GetName().c_str(),
        v.uid,
        lv->GetName().c_str()
    );
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
    RMGLog::OutFormat(
        RMGLog::debug,
        "Birks constant of material {} set to {} mm/MeV from GDML",
        mat->GetName(),
        bc / (CLHEP::mm / CLHEP::MeV)
    );
  }
}

void RMGHardware::StageDetector(
    RMGDetectorType type,
    const std::string& name,
    int uid,
    const std::string& copy_nr,
    bool allow_uid_reuse
) {
  if (fActiveDetectorsInitialized) {
    RMGLog::Out(RMGLog::error, "Active detectors cannot be mutated after constructing the detector.");
    return;
  }

  // sanity check for duplicate uids.
  // This would cause an error later, but we can catch it early.
  if (!allow_uid_reuse) {
    for (const auto& [k, v] : fStagedDetectors) {
      if (v.uid == uid) {
        RMGLog::OutFormat(RMGLog::error, "UID {} has already been assigned", uid);
        return;
      }
    }
  }

  auto r_value = fStagedDetectors.insert(
      {{name, copy_nr}, {type, name, uid, copy_nr, allow_uid_reuse}}
  );
  if (!r_value.second) { // if insertion did not take place
    RMGLog::OutFormat(
        RMGLog::warning,
        "Name '{}' (copy number {}) has already been staged as detector",
        name,
        copy_nr
    );
  }
}

void RMGHardware::RegisterDetector(
    RMGDetectorType type,
    const std::string& pv_name,
    int uid,
    int copy_nr,
    bool allow_uid_reuse
) {
  // This should not be possible to occur anymore
  if (fActiveDetectorsInitialized) {
    RMGLog::Out(RMGLog::error, "Active detectors cannot be mutated after constructing the detector.");
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
  auto r_value = fDetectorMetadata.insert({{pv_name, copy_nr}, {type, uid, pv_name, copy_nr}});
  if (!r_value.second) { // if insertion did not take place
    RMGLog::OutFormat(
        RMGLog::warning,
        "Physical volume '{}' (copy number {}) has already been registered as detector",
        pv_name,
        copy_nr
    );
  } else {
    RMGLog::OutFormat(
        RMGLog::detail,
        "Registered physical volume '{}' (copy nr. {}) as {} detector type (uid={})",
        pv_name,
        copy_nr,
        magic_enum::enum_name(type),
        uid
    );

    RMGIpc::SendIpcNonBlocking(
        RMGIpc::CreateMessage(
            "detector",
            fmt::format("{}\x1e{}\x1e{}", magic_enum::enum_name(type), uid, pv_name)
        )
    );
  }
}

void RMGHardware::SetMaxStepLimit(double max_step, std::string name) {

  fPhysVolStepLimits.insert_or_assign(name, max_step);

  RMGLog::OutFormat(RMGLog::detail, "Set step limits for {:s} to {:.2f} mm", name, max_step);
}

void RMGHardware::RegisterDetectorsFromGDML(std::string s) {
  if (s == "All") {
    for (const auto dt : magic_enum::enum_values<RMGDetectorType>()) {
      fRegisterDetectorsFromGDML.emplace(dt);
    }
  } else {
    fRegisterDetectorsFromGDML.emplace(RMGTools::ToEnum<RMGDetectorType>(s, "detector type"));
  }
}

void RMGHardware::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Geometry/",
      "Commands for controlling geometry definitions"
  );

  fMessenger->DeclareProperty("GDMLDisableOverlapCheck", fGDMLDisableOverlapCheck)
      .SetGuidance("Disable the automatic overlap check after loading a GDML file")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareProperty("GDMLOverlapCheckNumPoints", fGDMLOverlapCheckNumPoints)
      .SetGuidance("Change the number of points sampled for overlap checks")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("RegisterDetectorsFromGDML", &RMGHardware::RegisterDetectorsFromGDML)
      .SetGuidance(
          "Register detectors as saved in the GDML auxval structure, as written by pygeomtools."
      )
      .SetParameterName("det_type", true)
      .SetCandidates("All " + RMGTools::GetCandidates<RMGDetectorType>())
      .SetDefaultValue("All")
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("IncludeGDMLFile", &RMGHardware::IncludeGDMLFile)
      .SetGuidance("Use GDML file for geometry definition")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("PrintListOfLogicalVolumes", &RMGHardware::PrintListOfLogicalVolumes)
      .SetGuidance("Print list of defined logical volumes")
      .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("PrintListOfPhysicalVolumes", &RMGHardware::PrintListOfPhysicalVolumes)
      .SetGuidance("Print list of defined physical volumes")
      .SetStates(G4State_Idle);

  // RegisterDetector cannot be defined with the G4GenericMessenger (it has too many parameters).
  fHwMessenger = std::make_unique<RMGHardwareMessenger>(this);
}

// vim: tabstop=2 shiftwidth=2 expandtab

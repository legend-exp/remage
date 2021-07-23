#include "RMGManagementDetectorConstruction.hh"

#include <filesystem>
namespace fs = std::filesystem;

#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UserLimits.hh"
#include "G4GDMLParser.hh"

#include "RMGMaterialTable.hh"
#include "RMGLog.hh"
#include "RMGNavigationTools.hh"

RMGMaterialTable::BathMaterial RMGManagementDetectorConstruction::fBathMaterial = RMGMaterialTable::BathMaterial::kAir;

RMGManagementDetectorConstruction::RMGManagementDetectorConstruction() {
  fMaterialTable = std::make_unique<RMGMaterialTable>();
}

G4VPhysicalVolume* RMGManagementDetectorConstruction::Construct() {

  RMGLog::Out(RMGLog::debug, "Constructing detector");

  if (!fGDMLFiles.empty()) {
    RMGLog::Out(RMGLog::debug, "Setting up G4GDMLParser");
    G4GDMLParser parser;
    parser.SetOverlapCheck(true);
    for (const auto& file : fGDMLFiles) {
      RMGLog::Out(RMGLog::detail, "Reading ", file, " GDML file");
      if (!fs::exists(fs::path(file.data()))) RMGLog::Out(RMGLog::fatal, file, " does not exist");
      parser.Read(file);
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
    auto vol = RMGNavigationTools::FindVolumeByName(el.first);
    if (!vol) {
      RMGLog::Out(RMGLog::error, "Returned volume is null, skipping user step limit setting");
    }
    else vol->GetLogicalVolume()->SetUserLimits(new G4UserLimits(el.second));
  }

  return fWorld;
}

void RMGManagementDetectorConstruction::ConstructSDandField() {
  // TODO
}

void RMGManagementDetectorConstruction::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Geometry",
      "Commands for controlling geometry definitions");

  fMessenger->DeclareMethod("GDMLFile", &RMGManagementDetectorConstruction::IncludeGDMLFile)
    .SetGuidance("Use GDML file for geometry definition")
    .SetParameterName("filename", false)
    .SetToBeBroadcasted(false)
    .SetStates(G4State_PreInit, G4State_Init);
}

// vim: tabstop=2 shiftwidth=2 expandtab

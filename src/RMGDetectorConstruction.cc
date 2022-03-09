#include "RMGDetectorConstruction.hh"

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

RMGMaterialTable::BathMaterial RMGDetectorConstruction::fBathMaterial = RMGMaterialTable::BathMaterial::kAir;

RMGDetectorConstruction::RMGDetectorConstruction() {
  fMaterialTable = std::make_unique<RMGMaterialTable>();

  this->DefineCommands();
}

G4VPhysicalVolume* RMGDetectorConstruction::Construct() {

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
    auto vol = RMGNavigationTools::FindPhysicalVolume(el.first);
    if (!vol) {
      RMGLog::Out(RMGLog::error, "Returned volume is null, skipping user step limit setting");
    }
    else vol->GetLogicalVolume()->SetUserLimits(new G4UserLimits(el.second));
  }

  return fWorld;
}

void RMGDetectorConstruction::ConstructSDandField() {
  // TODO
}

void RMGDetectorConstruction::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Geometry/",
      "Commands for controlling geometry definitions");

  fMessenger->DeclareMethod("IncludeGDMLFile", &RMGDetectorConstruction::IncludeGDMLFile)
    .SetGuidance("Use GDML file for geometry definition")
    .SetParameterName("filename", false)
    .SetStates(G4State_PreInit);

  fMessenger->DeclareMethod("PrintListOfLogicalVolumes", &RMGDetectorConstruction::PrintListOfLogicalVolumes)
    .SetGuidance("Print list of defined physical volumes")
    .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("PrintListOfPhysicalVolumes", &RMGDetectorConstruction::PrintListOfPhysicalVolumes)
    .SetGuidance("Print list of defined physical volumes")
    .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "HPGeTestStand.hh"

#include "RMGMaterialTable.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

G4VPhysicalVolume* HPGeTestStand::DefineGeometry() {

    auto world_s = new G4Box("WorldLAr",
            0.5*CLHEP::m, 0.5*CLHEP::m, 0.5*CLHEP::m);

    auto world_l = new G4LogicalVolume(world_s,
            RMGMaterialTable::GetMaterial("LiquidArgon"),
            "WorldLAr");

    auto world_p = new G4PVPlacement(nullptr,
            G4ThreeVector(),
            world_l,
            "World",
            0, false, 0);

    auto hpge_s = new G4Box("HPGe",
            5*CLHEP::cm, 5*CLHEP::cm, 5*CLHEP::cm);

    auto hpge_l = new G4LogicalVolume(hpge_s,
            RMGMaterialTable::GetMaterial("EnrichedGermanium"),
            "HPGe");

    new G4PVPlacement(nullptr,
            G4ThreeVector(),
            hpge_l,
            "HPGe",
            world_l,
            false, 0);

    return world_p;
}

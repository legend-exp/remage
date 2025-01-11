#include <iostream>
#include <memory>
#include <vector>

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4VSolid.hh"
#include "Randomize.hh"

#include "RMGVertexConfinement.hh"

G4VPhysicalVolume* get_phy_volume(G4VSolid* solid, std::string Material_string) {
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* Material = nist->FindOrBuildMaterial(Material_string);
  G4LogicalVolume* logicalVolume = new G4LogicalVolume(solid, Material, "log_vol");

  G4ThreeVector position = G4ThreeVector(0, 0, 0); // Origin
  G4RotationMatrix* rotation = nullptr;
  G4PVPlacement* physicalVolume = new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), logicalVolume,
      "phy_vol", nullptr, false, 0);

  return physicalVolume;
}


int main(int argc, char* argv[]) {
  // Check if exactly one argument is provided
  std::string test_type = argv[1];

  if (test_type == "test-intersections-basic") {

    // make a basic geometry - first just a G4Tubs
    G4Tubs* tubby = new G4Tubs("tubby", 0 * mm, 50 * mm, 50 * mm, 0, 360 * deg);
    auto tubby_phy = get_phy_volume(tubby, "G4_SKIN_ICRP");
    RMGVertexConfinement::SampleableObject obj = RMGVertexConfinement::SampleableObject(tubby_phy,
        G4RotationMatrix(), G4ThreeVector(), nullptr);


    // shoot along x
    std::vector<G4ThreeVector> ints =
        obj.GetIntersections(G4ThreeVector(60 * mm, 0, 0), G4ThreeVector(-1, 0, 0));

    if (ints.size() != 2) {
      std::cout << "The number of intersections should be 2" << std::endl;
      return 1;
    }
    // shoot along z
    ints = obj.GetIntersections(G4ThreeVector(0 * mm, 0, 60 * mm), G4ThreeVector(0, 0, -1));

    if (ints.size() != 2) {
      std::cout << "The number of intersections should be 2" << std::endl;
      return 1;
    }

    // miss the object
    ints = obj.GetIntersections(G4ThreeVector(60 * mm, 0, 0), G4ThreeVector(0, -1, 0));

    if (ints.size() != 0) {
      std::cout << "The number of intersections should be 0" << std::endl;
      return 1;
    }

    // now generate a bunch randomly
    int i = 0;
    while (i < 10000) {
      G4ThreeVector dir;
      G4ThreeVector pos;
      obj.GetDirection(dir, pos);

      int ints = obj.GetIntersections(pos, dir).size();

      if (ints != 0 and ints != 2) {
        std::cout << "The number of intersections can only be 0 or 2 not " << ints << std::endl;
        return 1;
      }
      i++;
    }

    // tests passed
    return 0;


  } else if (test_type == "test-intersections-subtraction") {
    G4Tubs* tubby = new G4Tubs("tubby", 0 * mm, 50 * mm, 50 * mm, 0, 360 * deg);
    G4Tubs* small_tubby = new G4Tubs("small_tubby", 0 * mm, 10 * mm, 50 * mm, 0, 360 * deg);
    G4SubtractionSolid* subby = new G4SubtractionSolid("subby", tubby, small_tubby);
    auto subby_phy = get_phy_volume(subby, "G4_SKIN_ICRP");
    RMGVertexConfinement::SampleableObject obj = RMGVertexConfinement::SampleableObject(subby_phy,
        G4RotationMatrix(), G4ThreeVector(), nullptr);


    // shoot along x

    std::vector<G4ThreeVector> ints =
        obj.GetIntersections(G4ThreeVector(60 * mm, 0, 0), G4ThreeVector(-1, 0, 0));

    if (ints.size() != 4) {
      std::cout << "The number of intersections should be 4" << std::endl;
      return 1;
    }

    // shoot along z

    ints = obj.GetIntersections(G4ThreeVector(15 * mm, 0, 60 * mm), G4ThreeVector(0, 0, -1));

    if (ints.size() != 2) {
      std::cout << "The number of intersections should be 2" << std::endl;
      return 1;
    }

    int i = 0;
    while (i < 10000) {
      G4ThreeVector dir;
      G4ThreeVector pos;
      obj.GetDirection(dir, pos);

      int num_ints = obj.GetIntersections(pos, dir).size();

      if (num_ints != 0 and num_ints != 2 and num_ints != 4) {
        std::cout << "The number of intersections can only be 0, 2 or 4 not " << num_ints
                  << std::endl;
        return 1;
      }
      i++;
    }

  } else if (test_type == "test-containment") {
    G4Tubs* tubby = new G4Tubs("tubby", 0 * mm, 50 * mm, 50 * mm, 0, 360 * deg);
    G4Tubs* small_tubby = new G4Tubs("small_tubby", 0 * mm, 10 * mm, 50 * mm, 0, 360 * deg);
    G4SubtractionSolid* subby = new G4SubtractionSolid("subby", tubby, small_tubby);
    auto subby_phy = get_phy_volume(subby, "G4_SKIN_ICRP");
    RMGVertexConfinement::SampleableObject obj = RMGVertexConfinement::SampleableObject(subby_phy,
        G4RotationMatrix(), G4ThreeVector(), nullptr);

    int i = 0;
    while (i < 10000) {
      G4ThreeVector pos;
      bool success = obj.GenerateSurfacePoint(pos, 200, 4);

      if (subby->Inside(pos) != EInside::kSurface) {

        std::string side = (subby->Inside(pos) == EInside::kInside) ? "Inside" : "Outside";
        std::cout << "the sampled position is not inside the solid it is " << side << std::endl;
        return 1;
      }
      i++;
    }
  } else {
    return 0;
  }

  return 0;
}

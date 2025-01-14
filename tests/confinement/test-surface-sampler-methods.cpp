#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Orb.hh"
#include "G4PVPlacement.hh"
#include "G4RunManager.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4UnionSolid.hh"
#include "G4VSolid.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "G4VisExecutive.hh"
#include "Randomize.hh"

#include "RMGVertexConfinement.hh"

#include "QBBC.hh"

// class for some basic vis
class MyDetectorConstruction : public G4VUserDetectorConstruction {
  public:

    G4double fRadius;
    std::vector<G4ThreeVector> fPoints;
    G4LogicalVolume* fLog;
    G4ThreeVector fCenter;

    MyDetectorConstruction(G4ThreeVector center, G4double radius, std::vector<G4ThreeVector> points,
        G4LogicalVolume* solid_log) {
      fCenter = center;
      fRadius = radius;
      fPoints = points;
      fLog = solid_log;
    };

    G4VPhysicalVolume* Construct() override {
      // Define materials
      G4Material* vacuum = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

      // World volume
      G4Box* worldBox = new G4Box("World", 200 * mm, 200 * mm, 200 * mm);
      G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, vacuum, "World");

      // turn off vis
      auto worldVisAttr = new G4VisAttributes();
      worldVisAttr->SetVisibility(false);
      worldLog->SetVisAttributes(worldVisAttr);

      G4VPhysicalVolume* worldPhys =
          new G4PVPlacement(nullptr, {}, worldLog, "World", nullptr, false, 0);

      // Sampling sphere
      G4Orb* orb = new G4Orb("MyOrb", fRadius);
      G4LogicalVolume* orbLog = new G4LogicalVolume(orb, vacuum, "MyOrb");


      // place some solids
      new G4PVPlacement(nullptr, fCenter, orbLog, "MyOrb", worldLog, false, 0);
      new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), fLog, "sampled_solid", worldLog, false, 0);

      // Visualization attributes for the orb
      G4VisAttributes* orbVisAttr = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // Red
      orbVisAttr->SetVisibility(true);

      orbLog->SetVisAttributes(orbVisAttr);
      G4VisAttributes* VisAttr = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); // green
      VisAttr->SetVisibility(true);

      fLog->SetVisAttributes(VisAttr);

      // Create some G4ThreeVectors as points

      for (const auto& point : fPoints) {
        G4Sphere* pointSphere = new G4Sphere("Point", 0, 1 * mm, 0, 2 * M_PI, 0, M_PI);
        G4LogicalVolume* pointLog = new G4LogicalVolume(pointSphere, vacuum, "Point");
        new G4PVPlacement(nullptr, point, pointLog, "Point", worldLog, false, 0);

        // Visualization attributes for points

        G4VisAttributes* pointVisAttr = new G4VisAttributes(G4Colour(0, 0, 1)); // Blue
        pointVisAttr->SetVisibility(true);

        pointLog->SetVisAttributes(pointVisAttr);
      }

      return worldPhys;
    }
};


int RunVis(RMGVertexConfinement::SampleableObject obj,std::string name)
{
 G4double radius =
        obj.physical_volume->GetLogicalVolume()->GetSolid()->GetExtent().GetExtentRadius();
    G4ThreeVector center =
        obj.physical_volume->GetLogicalVolume()->GetSolid()->GetExtent().GetExtentCenter();

    G4LogicalVolume* log = obj.physical_volume->GetLogicalVolume();

    std::vector<G4ThreeVector> points;
    int i = 0;
    while (i < 100) {
      G4ThreeVector pos;
      G4ThreeVector dir;
      obj.GetDirection(dir, pos);

      points.push_back(pos);
      if ((pos - center).mag() < radius) {
        std::cout << "the initial point must be less than the bounding radius" << std::endl;
        return 1;
      }
      i++;
    }
    G4RunManager* runManager = new G4RunManager();

    // Set mandatory initialization classes
    runManager->SetUserInitialization(new MyDetectorConstruction(center, radius, points, log));

    auto physicsList = new QBBC;
    physicsList->SetVerboseLevel(1);
    runManager->SetUserInitialization(physicsList);
    runManager->Initialize();
    // Initialize visualization
    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();

    // User interface

    // Set up visualization commands
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    UImanager->ApplyCommand("/run/initialize ");

    UImanager->ApplyCommand("/vis/open OGL ");
    UImanager->ApplyCommand("/vis/verbose warnings");

    UImanager->ApplyCommand("/vis/drawVolume");
    UImanager->ApplyCommand("/vis/geometry/set/forceWireframe");
    UImanager->ApplyCommand("/vis/viewer/set/viewpointVector 0.7 0.9 0.7");
    UImanager->ApplyCommand("/vis/viewer/zoom 1.5");

    UImanager->ApplyCommand("/vis/scene/list ");
    UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");
    UImanager->ApplyCommand("/vis/viewer/set/globalLineWidthScale 1.5");
    UImanager->ApplyCommand("/vis/viewer/set/upVector 0 0 1");

    UImanager->ApplyCommand("/vis/ogl/export test-points-"+name+".output.pdf");


    delete visManager;
    delete runManager;


    return 0;


}

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

  // define the solids we need
  std::map<std::string, RMGVertexConfinement::SampleableObject> sampleables;


  // tubs
  G4Tubs* tubby = new G4Tubs("tubby", 0 * mm, 50 * mm, 50 * mm, 0, 360 * deg);
  auto tubby_phy = get_phy_volume(tubby, "G4_SKIN_ICRP");
  RMGVertexConfinement::SampleableObject obj_tub = RMGVertexConfinement::SampleableObject(tubby_phy,
      G4RotationMatrix(), G4ThreeVector(), nullptr);

  sampleables["tubs"] = obj_tub;

  G4Tubs* small_tubby = new G4Tubs("small_tubby", 0 * mm, 10 * mm, 50 * mm, 0, 360 * deg);
  G4SubtractionSolid* subby = new G4SubtractionSolid("subby", tubby, small_tubby);
  auto subby_phy = get_phy_volume(subby, "G4_SKIN_ICRP");
  RMGVertexConfinement::SampleableObject obj_sub = RMGVertexConfinement::SampleableObject(subby_phy,
      G4RotationMatrix(), G4ThreeVector(), nullptr);

  sampleables["sub"] = obj_sub;

  // box
  G4Box* box = new G4Box("box", 25 * mm, 25 * mm, 50 * mm);
  G4Box* small_box = new G4Box("small_box", 10 * mm, 10 * mm, 50 * mm);

  G4UnionSolid* solid =
      new G4UnionSolid("solid", box, small_box, nullptr, G4ThreeVector(0, 0, 60 * mm));
  auto union_phy = get_phy_volume(solid, "G4_SKIN_ICRP");
  RMGVertexConfinement::SampleableObject obj_box = RMGVertexConfinement::SampleableObject(union_phy,
      G4RotationMatrix(), G4ThreeVector(), nullptr);

  sampleables["uni"] = obj_box;

  if (test_type == "test-intersections-basic") {

    // make a basic geometry - first just a G4Tubs
    auto obj = sampleables["tubs"];

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

    auto obj = sampleables["sub"];
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

  }

  else if (test_type == "test-intersections-union") {


    auto obj = sampleables["uni"];

    // shoot along x

    std::vector<G4ThreeVector> ints =
        obj.GetIntersections(G4ThreeVector(90 * mm, 0, 0), G4ThreeVector(-1, 0, 0));

    if (ints.size() != 2) {
      std::cout << "The number of intersections should be 2" << std::endl;
      return 1;
    }

    // shoot along z
    ints = obj.GetIntersections(G4ThreeVector(15 * mm, 0, 95 * mm), G4ThreeVector(0, 0, -1));

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

  } else if (test_type == "test-containment-union") {
    auto obj = sampleables["uni"];

    int i = 0;
    while (i < 10000) {
      G4ThreeVector pos;
      bool success = obj.GenerateSurfacePoint(pos, 200, 4);

      if (obj.physical_volume->GetLogicalVolume()->GetSolid()->Inside(pos) != EInside::kSurface) {

        std::string side =
            (obj.physical_volume->GetLogicalVolume()->GetSolid()->Inside(pos) == EInside::kInside)
                ? "Inside"
                : "Outside";
        std::cout << "the sampled position is not inside the solid it is " << side << " " << pos
                  << std::endl;
        return 1;
      }
      i++;
    }
  } else if (test_type == "test-containment-subtraction") {
    auto obj = sampleables["sub"];

    int i = 0;
    while (i < 10000) {
      G4ThreeVector pos;
      bool success = obj.GenerateSurfacePoint(pos, 200, 4);

      if (obj.physical_volume->GetLogicalVolume()->GetSolid()->Inside(pos) != EInside::kSurface) {

        std::string side =
            (obj.physical_volume->GetLogicalVolume()->GetSolid()->Inside(pos) == EInside::kInside)
                ? "Inside"
                : "Outside";
        std::cout << "the sampled position is not inside the solid it is " << side << std::endl;
        return 1;
      }
      i++;
    }
  } else if (test_type == "test-points-union") {

    // get some points to plot
    auto obj = sampleables["uni"];
    RunVis(obj,"union");

  }
  else if (test_type == "test-points-subtraction") {

    // get some points to plot
    auto obj = sampleables["sub"];
    return RunVis(obj,"subtraction");

  } 
  else if (test_type == "test-points-basic") {

    // get some points to plot
    auto obj = sampleables["tubs"];
    return RunVis(obj,"simple");

  } 
  
   else {
    return 0;
  }

  return 0;
}

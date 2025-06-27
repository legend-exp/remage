#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4UImanager.hh"
#include "G4UserTrackingAction.hh"
#include "Randomize.hh"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

namespace u = CLHEP;

class TestGeometry : public RMGHardware {

  public:

    G4VPhysicalVolume* DefineGeometry() override {
      auto LAr = G4NistManager::Instance()->FindOrBuildMaterial("G4_lAr");

      auto world_s = new G4Box("world", 0.5 * u::m, 0.5 * u::m, 0.5 * u::m);
      auto world_l = new G4LogicalVolume(world_s, LAr, "world");
      auto world_p = new G4PVPlacement(nullptr, G4ThreeVector(), world_l, "world", nullptr, false, 0);

      return world_p;
    }
};

class G4Track;
class BremsTrackingAction : public G4UserTrackingAction {
  public:

    void PreUserTrackingAction(const G4Track*) override {
      auto name = CLHEP::HepRandom::getTheEngine()->name();
      if (name != "HepJamesRandom") {
        RMGLog::Out(RMGLog::error, "random engine not set on worker");
        std::abort();
      }
    }
};

int main(int argc, char** argv) {
  if (argc > 1) setenv("G4RUN_MANAGER_TYPE", argv[1], 1);

  RMGManager manager("test-random", argc, argv);
  manager.GetOutputManager()->EnablePersistency(false);
  manager.SetUserInit(new TestGeometry());
  manager.GetUserInit()->AddTrackingAction<BremsTrackingAction>();

  manager.Initialize();

  auto UI = G4UImanager::GetUIpointer();
  UI->ApplyCommand("/RMG/Manager/Randomization/Seed 1");
  UI->ApplyCommand("/RMG/Manager/Randomization/RandomEngine JamesRandom");
  UI->ApplyCommand("/run/initialize");
  UI->ApplyCommand("/RMG/Generator/Confine UnConfined");
  UI->ApplyCommand("/RMG/Generator/Select GPS");
  UI->ApplyCommand("/gps/particle                gamma");
  UI->ApplyCommand("/gps/energy                  2 MeV");
  UI->ApplyCommand("/gps/position                0 0 0");
  UI->ApplyCommand("/gps/ang/type                iso");
  UI->ApplyCommand("/run/beamOn 10");

  return 0;
}

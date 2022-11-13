#include "RMGLog.hh"
#include "RMGManager.hh"

#include "HPGeTestStand.hh"

int main(int argc, char** argv) {

  // RMGLog::SetLogLevel(RMGLog::debug);

  RMGManager manager("02-hpge", argc, argv);
  manager.SetUserInit(new HPGeTestStand());

  manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe1", 0);
  manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe2", 1);
  manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe3", 2);
  manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe4", 3);

  std::string macro = argc > 1 ? argv[1] : "";
  if (!macro.empty()) manager.IncludeMacroFile(macro);

  manager.Initialize();
  manager.Run();

  return 0;
}

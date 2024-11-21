#include "RMGLog.hh"
#include "RMGManager.hh"

#include "HPGeTestStand.hh"

int main(int argc, char** argv) {

  // RMGLog::SetLogLevel(RMGLog::debug);

  RMGManager manager("04-surface-cosmogenic", argc, argv);
  manager.SetUserInit(new HPGeTestStand());

  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe1", 0);
  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe2", 1);
  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe3", 2);
  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe4", 3);

  std::string macro = argc > 1 ? argv[1] : "";
  if (!macro.empty()) manager.IncludeMacroFile(macro);

  manager.Initialize();
  manager.Run();

  return 0;
}

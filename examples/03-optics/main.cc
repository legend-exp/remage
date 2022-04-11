#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

int main(int argc, char** argv) {

  RMGLog::SetLogLevel(RMGLog::debug);

  RMGManager manager("03-optics", argc, argv);
  manager.GetDetectorConstruction()->IncludeGDMLFile("gdml/geometry.gdml");
  manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kOptical, "Detector", 0);

  std::string macro = argc > 1 ? argv[1] : "";
  if (!macro.empty()) manager.IncludeMacroFile(macro);

  manager.Initialize();
  manager.Run();

  return 0;
}

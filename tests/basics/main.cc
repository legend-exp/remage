#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

int main(int argc, char** argv) {

  // RMGLog::SetLogLevel(RMGLog::debug);

  RMGManager manager("basic", argc, argv);
  // manager.SetInteractive(true);

  manager.GetDetectorConstruction()->IncludeGDMLFile("gdml/main.gdml");

  std::string macro = argc > 1 ? argv[1] : "";
  if (!macro.empty()) manager.IncludeMacroFile(macro);

  manager.Initialize();
  manager.Run();

  return 0;
}

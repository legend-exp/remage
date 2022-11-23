#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

int main(int argc, char** argv) {

  // RMGLog::SetLogLevel(RMGLog::debug);

  RMGManager manager("basic", argc, argv);
  manager.GetDetectorConstruction()->IncludeGDMLFile("gdml/geometry.gdml");

  std::string arg1 = argc > 1 ? argv[1] : "";
  std::string arg2 = argc > 2 ? argv[2] : "";
  std::string macro;
  if (arg1 == "-i") {
    manager.SetInteractive();
    if (!arg2.empty()) manager.IncludeMacroFile(arg2);
  } else if (!arg1.empty()) manager.IncludeMacroFile(arg1);

  manager.Initialize();
  manager.Run();

  return 0;
}

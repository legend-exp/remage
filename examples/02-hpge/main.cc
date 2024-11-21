#include "RMGLog.hh"
#include "RMGManager.hh"

#include "HPGeTestStand.hh"

int main(int argc, char** argv) {

  RMGManager manager("02-hpge", argc, argv);
  manager.SetUserInit(new HPGeTestStand());

  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe1", 0);
  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe2", 1);
  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe3", 2);
  manager.GetDetectorConstruction()->RegisterDetector(kGermanium, "HPGe4", 3);

  std::string macro;
  if (argc == 2) macro = argv[1];
  else if (argc == 3) macro = argv[2];
  else RMGLog::Out(RMGLog::fatal, "Bad command line options");

  if (argc > 1 and std::string(argv[1]) == "-i") { manager.SetInteractive(); }

  if (!macro.empty()) manager.IncludeMacroFile(macro);

  manager.Initialize();
  manager.Run();

  return 0;
}

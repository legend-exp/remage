#include "RMGLog.hh"
#include "RMGManager.hh"

#include "HPGeTestStand.hh"
#include "IsotopeOutputScheme.hh"

int main(int argc, char** argv) {

  // RMGLog::SetLogLevel(RMGLog::debug);

  RMGManager manager("TestNeutronCapture", argc, argv);
  manager.SetUserInit(new HPGeTestStand());

  auto user_init = manager.GetUserInit();
  user_init->AddOptionalOutputScheme<IsotopeOutputScheme>("IsotopeOutputScheme");

  // Need at least 1 active detector, otherwise persistency not enabled (even if manually setting it)
  manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe1", 0);
  // manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe2", 1);
  // manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe3", 2);
  // manager.GetDetectorConstruction()->RegisterDetector(RMGHardware::kGermanium, "HPGe4", 3);

  std::string macro = argc > 1 ? argv[1] : "";
  if (!macro.empty()) manager.IncludeMacroFile(macro);
  else manager.SetInteractive(true);

  manager.SetNumberOfThreads(1);
  manager.Initialize();
  manager.Run();

  return 0;
}

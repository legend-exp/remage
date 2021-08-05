#include "RMGManager.hh"
#include "RMGManagementDetectorConstruction.hh"
#include "RMGLog.hh"

int main(int argc, char** argv) {

    RMGLog::SetLogLevel(RMGLog::debug);

    RMGManager manager("01-gdml", argc, argv);
    manager.GetManagementDetectorConstruction()->IncludeGDMLFile("gdml/main.gdml");

    std::string macro = argc > 1 ? argv[1] : "";
    if (!macro.empty()) manager.IncludeMacroFile(macro);

    manager.Initialize();
    manager.Run();

    return 0;
}

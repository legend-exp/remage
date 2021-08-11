#include "RMGManager.hh"
#include "RMGLog.hh"

#include "DetectorConstruction.hh"

int main(int argc, char** argv) {

    RMGLog::SetLogLevel(RMGLog::debug);

    RMGManager manager("02-hpge", argc, argv);
    manager.SetUserInitialization(new DetectorConstruction());

    std::string macro = argc > 1 ? argv[1] : "";
    if (!macro.empty()) manager.IncludeMacroFile(macro);

    manager.Initialize();
    manager.Run();

    return 0;
}

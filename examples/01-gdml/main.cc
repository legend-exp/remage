#include "RMGManager.hh"
#include "RMGLog.hh"

int main(int argc, char** argv) {

    RMGLog::SetLogLevel(RMGLog::debug);

    RMGManager manager("01-gdml");

    auto status = manager.ParseCommandLineArgs(argc, argv);
    if (!status) return 1;

    manager.Initialize();
    manager.Run();

    return 0;
}

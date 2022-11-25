#include <vector>
#include <string>

#include "CLI11/CLI11.hpp"

#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"

int main(int argc, char** argv) {

  CLI::App app{"remage: simulation framework for germanium experiments"};

  int verbosity;
  bool quiet = false;
  int nthreads = 1;
  bool interactive = false;
  std::vector<std::string> gdmls;
  std::vector<std::string> macros;
  RMGLog::LogLevel loglevel = RMGLog::summary;

  app.add_flag("-q", quiet, "Print only warnings and errors");
  app.add_flag("-v", verbosity, "Verbosity");
  app.add_option("-l,--log-level", loglevel, "Logging level");

  app.add_flag("-i,--interactive", interactive, "Run in interactive mode");
  app.add_option("-t,--threads", nthreads, "Number of threads");
  app.add_option("-g,--gdml-files", gdmls, "GDML files");
  app.add_option("macros", macros, "Macro files");

  CLI11_PARSE(app, argc, argv);

  RMGLog::SetLogLevel(loglevel);

  switch (verbosity) {
    case 1: RMGLog::SetLogLevel(RMGLog::detail); break;
    case 2: RMGLog::SetLogLevel(RMGLog::debug); break;
    default: break;
  }

  if (quiet) RMGLog::SetLogLevel(RMGLog::warning);

  RMGManager manager("remage", argc, argv);
  manager.SetInteractive(interactive);
  manager.SetNumberOfThreads(nthreads);

  for (const auto& g : gdmls) manager.GetDetectorConstruction()->IncludeGDMLFile(g);
  for (const auto& m : macros) manager.IncludeMacroFile(m);

  manager.Initialize();
  manager.Run();

  return 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

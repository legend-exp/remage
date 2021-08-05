#ifndef _RMG_TOOLS_HH_
#define _RMG_TOOLS_HH_

#include <memory>
#include <vector>
#include <utility>
#include <ctime>
#include <chrono>

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4ApplicationState.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

namespace RMGTools {

  std::tm ToUTCTime(std::chrono::time_point<std::chrono::system_clock> t);

  template <typename T>
  T ToEnum(G4String);

}

#include "RMGTools.icc"

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

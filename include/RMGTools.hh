#ifndef _RMG_TOOLS_HH_
#define _RMG_TOOLS_HH_

#include <memory>
#include <vector>
#include <utility>
#include <ctime>
#include <chrono>

class G4String;
namespace RMGTools {

  template <typename T>
  T ToEnum(G4String);

  template <typename T>
  G4String GetCandidates();
}

#include "RMGTools.icc"

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

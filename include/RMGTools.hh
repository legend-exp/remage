#ifndef _RMG_TOOLS_HH_
#define _RMG_TOOLS_HH_

#include <memory>
#include <vector>
#include <utility>
#include <ctime>
#include <chrono>

namespace RMGTools {

  template <typename T>
  T ToEnum(std::string);

  template <typename T>
  std::string GetCandidates();
}

#include "RMGTools.icc"

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

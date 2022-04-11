#ifndef _RMG_TOOLS_HH_
#define _RMG_TOOLS_HH_

#include <chrono>
#include <ctime>
#include <memory>
#include <utility>
#include <vector>

namespace RMGTools {

  template<typename T> T ToEnum(std::string);

  template<typename T> std::string GetCandidates();
} // namespace RMGTools

#include "RMGTools.icc"

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

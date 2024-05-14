// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _RMG_TOOLS_HH_
#define _RMG_TOOLS_HH_

#include <stdexcept>

#include "globals.hh"

#include "RMGLog.hh"

#include "magic_enum/magic_enum.hpp"

namespace RMGTools {

  template<typename T> T ToEnum(const std::string name, std::string prop_name = "property") {
    auto result = magic_enum::enum_cast<T>(name);
    if (!result.has_value()) result = magic_enum::enum_cast<T>("k" + name);
    if (!result.has_value()) {
      RMGLog::OutFormat(RMGLog::error, "Illegal '{}' {} specified", name, prop_name);
      throw std::bad_cast();
    } else return result.value();
  }

  template<typename T> std::string GetCandidates(const char delim = ' ') {
    auto v = magic_enum::enum_names<T>();
    std::string cand;
    for (const auto& s : v) {
      auto name = s[0] == 'k' ? s.substr(1, std::string::npos) : s;
      cand += std::string(name) + delim;
    }
    return cand.substr(0, cand.size() - 1);
  }

  template<typename T> std::string GetCandidate(T t) {
    auto s = magic_enum::enum_name<T>(t);
    auto name = s[0] == 'k' ? s.substr(1, std::string::npos) : s;
    return std::string(name);
  }
} // namespace RMGTools

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

  /**
   * @brief Converts a @c std::string to an enumeration value.
   *
   * This function attempts to cast the provided @c std::string into an enum value of type T,
   * using the provided string (optionally with an added prefix @c "k").
   * If both attempts fail, an error is logged and a @c std::bad_cast is thrown.
   *
   * @tparam T The enumeration type.
   * @param name The @c std::string representing the enumeration value.
   * @param prop_name The property name used for logging if the conversion fails (default: @c "property").
   * @return The enum value corresponding to the given string.
   * @throws std::bad_cast If the conversion is not successful.
   */
  template<typename T> T ToEnum(const std::string name, std::string prop_name = "property") {
    auto result = magic_enum::enum_cast<T>(name);
    if (!result.has_value()) result = magic_enum::enum_cast<T>("k" + name);
    if (!result.has_value()) {
      RMGLog::OutFormat(RMGLog::error, "Illegal '{}' {} specified", name, prop_name);
      throw std::bad_cast();
    } else return result.value();
  }

  /**
   * @brief Generates a delimited list of candidate enumeration names.
   *
   * This function returns a @c std::string listing all the enumeration names for type T, separated by the
   * specified delimiter. If an enumeration name begins with @c 'k', that letter is removed.
   *
   * @tparam T The enumeration type.
   * @param delim The delimiter used to separate candidate names (default is a space).
   * @return A @c std::string containing the candidate enumeration names separated by the given delimiter.
   */
  template<typename T> std::string GetCandidates(const char delim = ' ') {
    auto v = magic_enum::enum_names<T>();
    std::string cand;
    for (const auto& s : v) {
      auto name = s[0] == 'k' ? s.substr(1, std::string::npos) : s;
      cand += std::string(name) + delim;
    }
    return cand.substr(0, cand.size() - 1);
  }

  /**
   * @brief Retrieves the candidate name for an enumeration value.
   *
   * This function obtains the @c std::string representation of the enumeration value \p t,
   * and removes a leading @c 'k' if present.
   *
   * @tparam T The enumeration type.
   * @param t The enumeration value.
   * @return A @c std::string representing the candidate name.
   */
  template<typename T> std::string GetCandidate(T t) {
    auto s = magic_enum::enum_name<T>(t);
    auto name = s[0] == 'k' ? s.substr(1, std::string::npos) : s;
    return std::string(name);
  }
} // namespace RMGTools

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

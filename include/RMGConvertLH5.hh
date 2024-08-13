// Copyright (C) 2024 Manuel Huber
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


#ifndef _RMG_CONVERT_LH5_HH
#define _RMG_CONVERT_LH5_HH

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#include "RMGLog.hh"

#include "H5Cpp.h"

class RMGConvertLH5 {

  public:

    static bool ConvertToLH5(std::string, bool);

  private:

    inline RMGConvertLH5(bool dry_run) : fDryRun(dry_run) {};

    bool ConvertToLH5Internal(std::string);

    static inline const std::regex names_split_re = std::regex("(_in_.+?)?\\0");
    static inline const std::regex names_split_re_end = std::regex("(_in_.+?)$");
    static inline const std::string log_prefix = "ConvertLH5: ";
    static inline const std::string log_prefix_dry = "ConvertLH5[dry-run]: ";

    static int iter_children(long int, const char*, const H5L_info_t*, void*);
    static std::vector<std::string> GetChildren(H5::Group&);

    void SetStringAttribute(H5::H5Object&, std::string, std::string);

    std::pair<std::string, std::vector<std::string>> ReadNullSepDataset(H5::Group&, std::string,
        std::string);

    H5::DataType FormToHDFDataType(std::string);
    std::string DataTypeToLGDO(H5::DataType);
    bool ConvertNTupleToTable(H5::Group&);

    bool CheckGeantHeader(H5::Group&);

    template<typename T, typename... Args>
    inline void LH5Log(RMGLog::LogLevel loglevel, const T& t, const Args&... args) {
      if (fDryRun && loglevel < RMGLog::error) return;
      RMGLog::Out(loglevel, fDryRun ? log_prefix_dry : log_prefix, t, args...);
    }

    bool fDryRun;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

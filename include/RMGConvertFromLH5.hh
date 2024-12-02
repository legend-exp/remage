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


#ifndef _RMG_CONVERT_FROM_LH5_HH
#define _RMG_CONVERT_FROM_LH5_HH

#include <filesystem>
#include <optional>
#include <regex>
#include <string>
#include <vector>

#include "RMGLog.hh"

#include "H5Cpp.h"

class RMGConvertFromLH5 {

  public:

    static bool ConvertFromLH5(std::string, std::string, bool, bool part_of_batch = false);

    inline static bool fIsStandalone = false;

  private:

    inline RMGConvertFromLH5(std::string filename, std::string ntuple_group, bool dry_run,
        bool part_of_batch)
        : fHdf5FileName(filename), fNtupleGroupName(ntuple_group), fDryRun(dry_run),
          fIsPartOfBatch(part_of_batch) {};

    bool ConvertFromLH5Internal();

    static int iter_children(long int, const char*, const H5L_info_t*, void*);
    static std::vector<std::string> GetChildren(H5::Group&);

    bool ExistsByType(H5::H5Location&, std::string, H5O_type_t);

    void SetStringAttribute(H5::H5Object&, std::string, std::string);
    std::optional<std::string> GetStringAttribute(H5::H5Object&, std::string);

    void CreateUIntDataset(H5::Group&, std::string, uint64_t);
    void CreateStringDataset(H5::Group&, std::string, std::string);

    std::string HDFDataTypeToForm(H5::DataType);
    bool ConvertTableToNTuple(H5::Group&);

    template<typename... Args> inline void LH5Log(RMGLog::LogLevel loglevel, const Args&... args) {
      if (fDryRun && loglevel < RMGLog::error) return;
      std::string fn_prefix = fIsPartOfBatch ? " (" + fHdf5FileName + ")" : "";
      RMGLog::Out(loglevel, "", fIsStandalone ? "" : "ConvertLH5", fDryRun ? "[dry-run]" : "",
          fn_prefix, ": ", args...);
    }

    std::string fHdf5FileName;
    std::string fNtupleGroupName;
    bool fDryRun;
    bool fIsPartOfBatch;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

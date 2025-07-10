// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "RMGLog.hh"

#include "H5Cpp.h"

/**
 * @brief Converter class for converting between HDF5 and LH5 file formats.
 *
 * This class provides methods to convert Geant4 HDF5 output files into the LH5 format
 * and also to convert LH5 input files back to HDF5. It supports conversion of ntuple data.
 */
class RMGConvertLH5 {

  public:

    /**
     * @brief Convert a Geant4 HDF5 output file to LH5 format.
     *
     * This function converts the specified HDF5 file to the LH5 format, reformatting
     * the ntuple data into table format.
     *
     * @param hdf5_file_name The input HDF5 file name.
     * @param ntuple_group_name The name of the ntuple group in the HDF5 file.
     * @param aux_ntuples A set of auxiliary ntuple names.
     * @param ntuple_meta A mapping of detector uids to a pair of ntuple id and ntuple name.
     * @param dry_run If true, the conversion is performed in-memory without writing to disk.
     * @param part_of_batch Indicates if this conversion is part of a batch operation.
     *
     * @return True if the conversion is successful, false otherwise.
     */
    static bool ConvertToLH5(
        std::string,
        std::string,
        std::set<std::string>,
        const std::map<int, std::pair<int, std::string>>&,
        bool,
        bool part_of_batch = false
    );
    /**
     * @brief Convert an LH5 input file to HDF5 format.
     *
     * This function converts the specified LH5 file to HDF5 format suitable for reading
     * by Geant4, reformatting ntuple tables into ntuple datasets.
     *
     * @param lh5_file_name The input LH5 file name.
     * @param ntuple_group_name The name of the ntuple group in the LH5 file.
     * @param dry_run If true, the conversion is performed in-memory without writing to disk.
     * @param part_of_batch Indicates if this conversion is part of a batch operation.
     * @param units_map Output parameter: a mapping from ntuple names to their unit dictionaries.
     *
     * @return True if the conversion is successful, false otherwise.
     */
    static bool ConvertFromLH5(
        std::string,
        std::string,
        bool,
        bool part_of_batch,
        std::map<std::string, std::map<std::string, std::string>>&
    );

    inline static bool fIsStandalone = false;

  private:

    RMGConvertLH5(
        std::string filename,
        std::string ntuple_group,
        std::set<std::string> aux_ntuples,
        const std::map<int, std::pair<int, std::string>>& ntuple_meta,
        bool dry_run,
        bool part_of_batch
    )
        : fHdf5FileName(filename), fNtupleGroupName(ntuple_group), fAuxNtuples(aux_ntuples),
          fNtupleMeta(ntuple_meta), fDryRun(dry_run), fIsPartOfBatch(part_of_batch) {};

    ////////////////////////////////////////////////////////////////////////////////////////////

    static int iter_children(hid_t, const char*, const H5L_info_t*, void*);
    static std::vector<std::string> GetChildren(H5::Group&);

    bool ExistsByType(H5::H5Location&, std::string, H5O_type_t);

    void SetStringAttribute(H5::H5Object&, std::string, std::string);
    std::optional<std::string> GetStringAttribute(H5::H5Object&, std::string);

    void CreateUIntDataset(H5::Group&, std::string, uint64_t);
    void CreateStringDataset(H5::Group&, std::string, std::string);

    ////////////////////////////////////////////////////////////////////////////////////////////
    // HDF5 -> LH5 (output files):

    bool ConvertToLH5Internal();

    std::pair<std::string, std::vector<std::string>> ReadNullSepDataset(
        H5::Group&,
        std::string,
        std::string
    );

    static inline const std::regex names_it_re = std::regex("(_in_.+?)?\\0|(_in_.+?)$");

    std::unique_ptr<H5::DataType> FormToHDFDataType(std::string);
    std::string DataTypeToLGDO(H5::DataType);
    bool ConvertNTupleToTable(H5::Group&);

    bool CheckGeantHeader(H5::Group&);

    ////////////////////////////////////////////////////////////////////////////////////////////
    // LH5 -> HDF5 (input files):

    bool ConvertFromLH5Internal(std::map<std::string, std::map<std::string, std::string>>&);

    static inline const std::regex table_dtype_re = std::regex("^table\\{.*\\}$");

    std::string HDFDataTypeToForm(H5::DataType);
    bool ConvertTableToNTuple(H5::Group&, std::map<std::string, std::string>&);

    ////////////////////////////////////////////////////////////////////////////////////////////

    template<typename... Args> void LH5Log(RMGLog::LogLevel loglevel, const Args&... args) {
      if (fDryRun && loglevel < RMGLog::error) return;
      std::string fn_prefix = fIsPartOfBatch ? " (" + fHdf5FileName + ")" : "";
      RMGLog::Out(
          loglevel,
          "",
          fIsStandalone ? "" : "ConvertLH5",
          fDryRun ? "[dry-run]" : "",
          fn_prefix,
          ": ",
          args...
      );
    }

    std::string fHdf5FileName;
    std::string fNtupleGroupName;
    std::set<std::string> fAuxNtuples;
    std::map<int, std::pair<int, std::string>> fNtupleMeta;
    std::string fUIDKeyFormatString = "det{:03}";
    bool fDryRun;
    bool fIsPartOfBatch;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include "RMGConvertLH5.hh"

#include <cstdint>
#include <fmt/ranges.h>
#include <regex>
#include <string>
#include <vector>

#include "RMGIpc.hh"
#include "RMGLog.hh"

////////////////////////////////////////////////////////////////////////////////////////////

int RMGConvertLH5::iter_children(hid_t, const char* name, const H5L_info_t*, void* op_data) {
  auto ntuples = static_cast<std::vector<std::string>*>(op_data);
  ntuples->push_back(name);
  return 0;
}

std::vector<std::string> RMGConvertLH5::GetChildren(H5::Group& group) {
  std::vector<std::string> children;
  H5Literate(group.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, iter_children, &children);
  return children;
}

bool RMGConvertLH5::ExistsByType(H5::H5Location& loc, std::string name, H5O_type_t type) {
  return loc.nameExists(name) && loc.childObjType(name) == type;
}

void RMGConvertLH5::SetStringAttribute(H5::H5Object& obj, std::string attr_name, std::string attr_value) {
  H5::StrType att_dtype(0, H5T_VARIABLE);
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createAttribute(attr_name, att_dtype, scalar);
  att.write(att_dtype, attr_value);
}

std::optional<std::string> RMGConvertLH5::GetStringAttribute(H5::H5Object& obj, std::string attr_name) {
  if (!obj.attrExists("datatype")) return std::nullopt;
  auto att_writer = obj.openAttribute(attr_name);
  if (att_writer.getDataType().getClass() != H5T_STRING) return std::nullopt;
  std::string writer;
  att_writer.read(att_writer.getDataType(), writer);
  return writer;
}

void RMGConvertLH5::CreateUIntDataset(H5::Group& obj, std::string dset_name, uint64_t attr_value) {
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createDataSet(dset_name, H5::PredType::STD_I64LE, scalar);
  att.write(&attr_value, H5::PredType::STD_I32LE);
}

void RMGConvertLH5::CreateStringDataset(H5::Group& obj, std::string dset_name, std::string attr_value) {
  H5::StrType att_dtype(0, attr_value.size() + 1);
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createDataSet(dset_name, att_dtype, scalar);
  att.write(attr_value, att_dtype);
}

////////////////////////////////////////////////////////////////////////////////////////////
// HDF5 -> LH5 (output files):

std::pair<std::string, std::vector<std::string>> RMGConvertLH5::ReadNullSepDataset(
    H5::Group& det_group,
    std::string dset_name,
    std::string ntuple_log_prefix
) {
  std::vector<std::string> vec;

  auto dset = det_group.openDataSet(dset_name);
  if (dset.getDataType().getClass() != H5T_STRING) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "invalid ", dset_name, " dataset");
    return {"", vec};
  }

  // note: the std::string-based API does not read the full string, because geant4 uses the wrong
  // data type (nul-terminated string vs. nul-padded string).
  size_t storage_size = dset.getStorageSize();
  char* buf = new char[storage_size];
  dset.read(buf, dset.getStrType());

  // the last two bytes of the buffer will be NUL.
  if (storage_size < 2 || buf[storage_size - 1] != '\0' || buf[storage_size - 2] != '\0') {
    LH5Log(RMGLog::error, ntuple_log_prefix, "invalid ", dset_name, " dataset");
    return {"", vec};
  }
  std::string s(buf, storage_size - 2);

  // split on null bytes.
  std::istringstream is(s);
  std::string tmp;
  while (std::getline(is, tmp, '\0')) vec.push_back(tmp);

  return {s, vec}; // strip the first NUL byte.
}

std::unique_ptr<H5::DataType> RMGConvertLH5::FormToHDFDataType(std::string form_type) {
  if (form_type == "string") return std::make_unique<H5::StrType>(0, H5T_VARIABLE);
  if (form_type == "int") return std::make_unique<H5::DataType>(H5::PredType::STD_I32LE);
  if (form_type == "float") return std::make_unique<H5::DataType>(H5::PredType::IEEE_F32LE);
  if (form_type == "double") return std::make_unique<H5::DataType>(H5::PredType::IEEE_F64LE);
  throw std::logic_error("not implemented form_type, LGDO will be invalid");
}

std::string RMGConvertLH5::DataTypeToLGDO(H5::DataType dtype) {
  auto dtype_class = dtype.getClass();
  switch (dtype_class) {
    case H5T_STRING: return "string";
    case H5T_INTEGER:
    case H5T_FLOAT: return "real";
    default: throw std::logic_error("not implemented H5T type, ntuple will be invalid");
  }
}

bool RMGConvertLH5::ConvertNTupleToTable(H5::Group& det_group) {
  const std::string ntuple_name = det_group.getObjName();
  const std::string ntuple_log_prefix = "ntuple " + ntuple_name + " - ";
  LH5Log(RMGLog::detail, ntuple_log_prefix, "visiting");

  if (!ExistsByType(det_group, "names", H5O_TYPE_DATASET) ||
      !ExistsByType(det_group, "forms", H5O_TYPE_DATASET) ||
      !ExistsByType(det_group, "columns", H5O_TYPE_DATASET)) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "missing names, forms or columns dataset");
    return false;
  }

  if (det_group.attrExists("datatype") || det_group.attrExists("units")) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "to-be-written LGDO attributes already exist");
    return false;
  }

  // read to check later if we received the correct column count.
  auto dset_columns = det_group.openDataSet("columns");
  if (dset_columns.getDataType() != H5::PredType::STD_U32LE) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "invalid columns dataset");
    return false;
  }
  uint32_t expected_column_count = 0;
  dset_columns.read(&expected_column_count, dset_columns.getDataType());

  // read the column names into a buffer. This contains the full data with null bytes.
  auto [names_string, names_parts] = ReadNullSepDataset(det_group, "names", ntuple_name);
  // "forms" contains the datatypes of Geant.
  auto [forms_string, forms_parts] = ReadNullSepDataset(det_group, "forms", ntuple_name);

  if (names_string.empty() || forms_string.empty()) return false;
  if (names_parts.size() != forms_parts.size()) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "mismatch in forms and names count");
    return false;
  }

  std::sregex_token_iterator it(names_string.begin(), names_string.end(), names_it_re, -1);
  std::vector<std::string> table_columns;
  std::copy(it, std::sregex_token_iterator(), std::back_inserter(table_columns));
  std::sort(table_columns.begin(), table_columns.end());
  // set the table lgdo datatype.
  SetStringAttribute(
      det_group,
      "datatype",
      "table{" + fmt::format("{}", fmt::join(table_columns, ",")) + "}"
  );

  // unlink the ntuple definition datasets and attributes.
  det_group.unlink("names");
  det_group.unlink("forms");
  det_group.unlink("columns");
  if (det_group.nameExists("entries"))
    det_group.unlink("entries"); // entry count; not relevant here.

  if (det_group.attrExists("class")) det_group.removeAttr("class");
  if (det_group.attrExists("type")) det_group.removeAttr("type");
  if (det_group.attrExists("version")) det_group.removeAttr("version");

  // iterate over the remaining child groups and update to LH5 format.
  auto columns = GetChildren(det_group);
  for (auto& column : columns) {
    // first check that we will not get name clashes later on.
    if (column.find("__tmp") != std::string::npos) {
      LH5Log(RMGLog::error, ntuple_log_prefix, "already containing temporary column ", column);
      return false;
    }
  }
  uint32_t out_column_count = 0;
  for (auto& column : columns) {
    auto lgdo_name = column;
    std::string lgdo_units;
    auto unit_sep_pos = column.rfind("_in_");
    if (unit_sep_pos != std::string::npos) {
      lgdo_name = column.substr(0, unit_sep_pos);
      lgdo_units = column.substr(unit_sep_pos + strlen("_in_"));
      // transform back the units, as they unfortunately cannot contain a forward slash in geant.
      std::replace(lgdo_units.begin(), lgdo_units.end(), '\\', '/');
    }

    LH5Log(RMGLog::detail, ntuple_log_prefix, "column ", lgdo_name, ", with units ", lgdo_units);

    // remove the column group with its child dataset, while preserving the data itself.
    std::string column_tmp = column + "__tmp";
    det_group.moveLink(column, column_tmp);
    if (det_group.nameExists(column_tmp + "/pages")) {
      det_group.moveLink(column_tmp + "/pages", lgdo_name);
    } else {
      // create a new empty dataset, as we have none.
      auto col_idx = std::distance(
          names_parts.begin(),
          std::find(names_parts.begin(), names_parts.end(), column)
      );
      LH5Log(
          RMGLog::warning,
          ntuple_log_prefix,
          "column ",
          lgdo_name,
          " - no data, creating with type ",
          forms_parts[col_idx]
      );

      hsize_t dset_dataspace_dim[1] = {0};
      H5::DataSpace dset_dataspace(1, dset_dataspace_dim);
      auto dset_dtype = FormToHDFDataType(forms_parts[col_idx]);
      det_group.createDataSet(lgdo_name, *dset_dtype, dset_dataspace);
    }
    det_group.unlink(column_tmp);

    auto dset_column = det_group.openDataSet(lgdo_name);
    if (unit_sep_pos != std::string::npos) { SetStringAttribute(dset_column, "units", lgdo_units); }
    // note: this simple decision only works as G4 can only write float/double/int/string.
    // Other types with a distinct lgdo type are not possible to reach here.
    auto lgdo_dtype = DataTypeToLGDO(dset_column.getDataType());
    SetStringAttribute(dset_column, "datatype", "array<1>{" + lgdo_dtype + "}");
    dset_column.close();

    out_column_count++;
  }

  if (out_column_count != expected_column_count || out_column_count != names_parts.size()) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "column count mismatch");
    return false;
  }
  return true;
}

bool RMGConvertLH5::CheckGeantHeader(H5::Group& header_group) {
  // validate that we have a valid geant4-generated HDF5 file header.

  if (!header_group.attrExists("data_schema_version")) return false;
  auto att_schema_version = header_group.openAttribute("data_schema_version");
  if (att_schema_version.getDataType() != H5::PredType::STD_I32LE) return false;
  int32_t schema_version = 0;
  att_schema_version.read(att_schema_version.getDataType(), &schema_version);
  if (schema_version != 1) return false;

  if (!header_group.attrExists("writer")) return false;
  auto att_writer = header_group.openAttribute("writer");
  if (att_writer.getDataType().getClass() != H5T_STRING) return false;
  std::string writer;
  att_writer.read(att_writer.getDataType(), writer);
  return writer == "exlib";
}

bool RMGConvertLH5::ConvertToLH5Internal() {
  // using the core driver with no backing storage will allow to change the file purely in-memory.
  // warning: this will internally allocate approx. the full file size!
  H5::FileAccPropList fapl;
  if (fDryRun) fapl.setCore(10 * 1024, false);
  else fapl = H5::FileAccPropList::DEFAULT;

  H5::H5File hfile(fHdf5FileName, H5F_ACC_RDWR, H5::FileCreatPropList::DEFAULT, fapl);

  auto ntuple_group_name = fNtupleGroupName;

  // check that this file has been written by geant4/remage, and that we did not run this upgrade
  // script before (it will delete the header group below).
  if (!ExistsByType(hfile, "header", H5O_TYPE_GROUP) ||
      !ExistsByType(hfile, ntuple_group_name, H5O_TYPE_GROUP)) {
    LH5Log(
        RMGLog::error,
        "not a remage HDF5 output file or already converted (missing header or ",
        ntuple_group_name,
        " groups)?"
    );
    return false;
  }
  auto header_group = hfile.openGroup("header");
  if (!CheckGeantHeader(header_group)) {
    LH5Log(RMGLog::error, "not a remage HDF5 output file (invalid header)?");
    return false;
  }
  LH5Log(RMGLog::detail, "opened Geant4 HDF5 file ", fHdf5FileName);

  // rework the ntuples to LGDO tables.
  auto ntuples_group = hfile.openGroup(ntuple_group_name);
  auto ntuples = GetChildren(ntuples_group);
  bool ntuple_success = true;

  std::string links_group_name = "__by_uid__";
  std::vector<std::string> links;
  RMGIpc::SendIpcNonBlocking(RMGIpc::CreateMessage("lh5_links_group_name", links_group_name));

  for (auto& ntuple : ntuples) {
    if (ntuple.empty()) LH5Log(RMGLog::fatal, "empty ntuple name, how is this possible?");

    auto det_group = ntuples_group.openGroup(ntuple);
    ntuple_success &= ConvertNTupleToTable(det_group);
    det_group.close();

    // Check fNtupleMeta object for an entry whose second field matches the current ntuple name.
    for (const auto& item : fNtupleMeta) {
      // item is a pair: item.first is an int, item.second is a std::pair<int, std::string>
      if (item.second.second == ntuple) {
        // create group named "links"
        if (!ExistsByType(ntuples_group, links_group_name, H5O_TYPE_GROUP)) {
          auto links_group = ntuples_group.createGroup(links_group_name);
          links_group.close();
        }

        // form soft link name "detUID" where UID is item.second.first.
        auto soft_link_name = fmt::format(fUIDKeyFormatString, item.first);
        auto soft_link_name_rel = std::string(links_group_name).append("/").append(soft_link_name);
        // do not create if the soft link already exists.
        if (!ntuples_group.nameExists(soft_link_name_rel)) {
          // create a soft link to the current group itself.
          ntuples_group.link(
              H5L_TYPE_SOFT,
              std::string("/").append(ntuple_group_name).append("/").append(ntuple),
              soft_link_name_rel
          );
          links.push_back(soft_link_name);
          LH5Log(RMGLog::detail, "created soft link ", ntuple_group_name, "/", soft_link_name_rel);
        }
        break;
      }
    }

    // if this is an auxiliary table, move it one level up out of the group
    if (fAuxNtuples.find(ntuple) != fAuxNtuples.end()) {
      LH5Log(RMGLog::debug, "moving ntuple ", ntuple_group_name, "/", ntuple, " one group back");
      hfile.moveLink(std::string(ntuple_group_name).append("/").append(ntuple), ntuple);
    }
  }

  // remove aux ntuples from list of ntuples in the stp/ group
  for (auto& ntuple : fAuxNtuples)
    ntuples.erase(std::remove(ntuples.begin(), ntuples.end(), ntuple), ntuples.end());

  // if the stp group is empty, remove it
  if (ntuples.empty()) hfile.unlink(ntuple_group_name);
  else {
    // make the root HDF5 group an LH5 struct.
    if (!ntuples_group.attrExists("datatype")) {
      LH5Log(RMGLog::debug, "making the root HDF5 group an LH5 struct");
      std::sort(ntuples.begin(), ntuples.end());
      SetStringAttribute(
          ntuples_group,
          "datatype",
          "struct{" + fmt::format("{}", fmt::join(ntuples, ",")) + "}"
      );
    }
  }

  // make links group an LH5 struct
  if (ExistsByType(ntuples_group, links_group_name, H5O_TYPE_GROUP)) {
    auto links_group = ntuples_group.openGroup(links_group_name);
    LH5Log(RMGLog::debug, "making the links HDF5 group an LH5 struct");
    std::sort(links.begin(), links.end());
    SetStringAttribute(
        links_group,
        "datatype",
        "struct{" + fmt::format("{}", fmt::join(links, ",")) + "}"
    );
    links_group.close();
  }

  if (ntuples_group.attrExists("type")) ntuples_group.removeAttr("type");

  // check other things that geant4 might write into the file, and delete them if they are empty.
  if (ExistsByType(hfile, "default_histograms", H5O_TYPE_GROUP)) {
    auto histo_group = hfile.openGroup("default_histograms");
    auto histograms = GetChildren(histo_group);
    if (histograms.empty()) {
      hfile.unlink("default_histograms");
    } else {
      LH5Log(RMGLog::warning, "HDF5 file contains histograms, not yet supported to convert!");
    }
  }

  // remove old header, so that we cannot run this conversion function again.
  hfile.unlink("header");

  hfile.close();

  LH5Log(RMGLog::summary, "Done updating HDF5 file ", fHdf5FileName, " to LH5");

  return ntuple_success;
}

bool RMGConvertLH5::ConvertToLH5(
    std::string hdf5_file_name,
    std::string ntuple_group_name,
    std::set<std::string> aux_ntuples,
    const std::map<int, std::pair<int, std::string>>& ntuple_meta,
    bool dry_run,
    bool part_of_batch
) {
  auto conv = RMGConvertLH5(
      hdf5_file_name,
      ntuple_group_name,
      aux_ntuples,
      ntuple_meta,
      dry_run,
      part_of_batch
  );
  try {
    return conv.ConvertToLH5Internal();
  } catch (const H5::Exception& e) {
    conv.LH5Log(RMGLog::error, e.getDetailMsg());
    return false;
  } catch (const std::logic_error& e) {
    conv.LH5Log(RMGLog::error, e.what());
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
// LH5 -> HDF5 (input files):

std::string RMGConvertLH5::HDFDataTypeToForm(H5::DataType dtype) {
  auto dtype_class = dtype.getClass();
  switch (dtype_class) {
    case H5T_STRING: return "string";
    case H5T_INTEGER: return "int";
    case H5T_FLOAT: return dtype.getSize() == 8 ? "double" : "float";
    default: throw std::logic_error("not implemented H5T type, LGDO will be invalid");
  }
}

bool RMGConvertLH5::ConvertTableToNTuple(
    H5::Group& det_group,
    std::map<std::string, std::string>& units_map
) {
  const std::string ntuple_name = det_group.getObjName();
  const std::string ntuple_log_prefix = "ntuple " + ntuple_name + " - ";
  LH5Log(RMGLog::detail, ntuple_log_prefix, "visiting");

  auto datatype_opt = GetStringAttribute(det_group, "datatype");
  if (!datatype_opt) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "not an LGDO");
    return false;
  }
  det_group.removeAttr("datatype");
  auto datatype = datatype_opt.value();

  std::smatch re_match;
  if (datatype.empty() || !std::regex_match(datatype, re_match, table_dtype_re)) {
    LH5Log(RMGLog::error, ntuple_log_prefix, "invalid LGDO datatype");
    return false;
  }

  // iterate over the remaining child groups and update to LH5 format.
  auto columns = GetChildren(det_group);
  for (auto& column : columns) {
    // first check that we will not get name clashes later on.
    if (column.find("__tmp") != std::string::npos) {
      LH5Log(RMGLog::error, ntuple_log_prefix, "already containing temporary column ", column);
      return false;
    }
  }
  std::string names_string, forms_string;
  uint64_t out_column_count = 0;
  uint64_t out_entries_count = 0;
  for (auto& lgdo_name : columns) {
    auto column = lgdo_name;

    auto dset_column = det_group.openDataSet(lgdo_name);
    if (dset_column.attrExists("units")) {
      units_map[lgdo_name] = GetStringAttribute(dset_column, "units").value();
    } else {
      units_map[lgdo_name] = "";
    }
    names_string += column + std::string("\0", 1);
    forms_string += HDFDataTypeToForm(dset_column.getDataType()) + std::string("\0", 1);
    dset_column.close();

    LH5Log(RMGLog::detail, ntuple_log_prefix, "column ", lgdo_name, " to ", column);

    // move the column to a pages array.
    std::string column_tmp = column + "__tmp";
    det_group.moveLink(lgdo_name, column_tmp);
    auto col_group = det_group.createGroup(column);
    det_group.moveLink(column_tmp, column + "/pages");

    dset_column = det_group.openDataSet(column + "/pages");
    auto dset_space = dset_column.getSpace();
    if (!dset_space.isSimple() || dset_space.getSimpleExtentNdims() != 1) {
      LH5Log(RMGLog::error, ntuple_log_prefix, "no simple dataspace for column ", column);
      return false;
    }
    hsize_t dims[1];
    dset_space.getSimpleExtentDims(dims);
    CreateUIntDataset(det_group, column + "/entries", dims[0]);
    if (out_column_count == 0) {
      out_entries_count = dims[0];
    } else if (out_entries_count != dims[0]) {
      LH5Log(
          RMGLog::error,
          ntuple_log_prefix,
          "mismatch for entry count for column ",
          column,
          " ",
          out_entries_count,
          " vs ",
          dims[0]
      );
      return false;
    }
    dset_column.removeAttr("datatype");
    dset_column.close();

    out_column_count++;
  }

  // set the table metadata datasets.
  CreateStringDataset(det_group, "forms", forms_string);
  CreateStringDataset(det_group, "names", names_string);
  CreateUIntDataset(det_group, "columns", out_column_count);
  CreateUIntDataset(det_group, "entries", out_entries_count);

  return true;
}

bool RMGConvertLH5::ConvertFromLH5Internal(
    std::map<std::string, std::map<std::string, std::string>>& units_map
) {
  if (!fAuxNtuples.empty()) {
    LH5Log(RMGLog::fatal, "Handling of auxiliary ntuples is not implemented yet");
  }

  // using the core driver with no backing storage will allow to change the file purely in-memory.
  // warning: this will internally allocate approx. the full file size!
  H5::FileAccPropList fapl;
  if (fDryRun) fapl.setCore(10 * 1024, false);
  else fapl = H5::FileAccPropList::DEFAULT;

  H5::H5File hfile(fHdf5FileName, H5F_ACC_RDWR, H5::FileCreatPropList::DEFAULT, fapl);

  auto ntuple_group_name = fNtupleGroupName;

  if (!ExistsByType(hfile, ntuple_group_name, H5O_TYPE_GROUP)) {
    LH5Log(RMGLog::error, "group ", ntuple_group_name, " is missing");
    return false;
  }
  LH5Log(RMGLog::detail, "Opened LH5 file ", fHdf5FileName);

  // rework the LGDO tables to ntuples.
  auto ntuples_group = hfile.openGroup(ntuple_group_name);
  auto ntuples = GetChildren(ntuples_group);
  bool ntuple_success = true;
  for (auto& ntuple : ntuples) {
    auto det_group = ntuples_group.openGroup(ntuple);
    units_map[ntuple] = {};
    ntuple_success &= ConvertTableToNTuple(det_group, units_map[ntuple]);
  }

  if (ntuples_group.attrExists("datatype")) ntuples_group.removeAttr("datatype");

  hfile.close();

  LH5Log(RMGLog::summary, "Done updating HDF5 file ", fHdf5FileName, " to LH5");

  return ntuple_success;
}

bool RMGConvertLH5::ConvertFromLH5(
    std::string lh5_file_name,
    std::string ntuple_group_name,
    bool dry_run,
    bool part_of_batch,
    std::map<std::string, std::map<std::string, std::string>>& units_map
) {
  auto conv = RMGConvertLH5(lh5_file_name, ntuple_group_name, {}, {}, dry_run, part_of_batch);
  try {
    return conv.ConvertFromLH5Internal(units_map);
  } catch (const H5::Exception& e) {
    conv.LH5Log(RMGLog::error, e.getDetailMsg());
    return false;
  } catch (const std::logic_error& e) {
    conv.LH5Log(RMGLog::error, e.what());
    return false;
  }
}


// vim: tabstop=2 shiftwidth=2 expandtab

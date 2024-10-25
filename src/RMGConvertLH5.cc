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

#include "RMGConvertLH5.hh"

#include <cstdint>
#include <regex>
#include <string>
#include <vector>

#include "RMGLog.hh"

int RMGConvertLH5::iter_children(long int, const char* name, const H5L_info_t*, void* op_data) {
  auto ntuples = static_cast<std::vector<std::string>*>(op_data);
  ntuples->push_back(name);
  return 0;
}

std::vector<std::string> RMGConvertLH5::GetChildren(H5::Group& group) {
  std::vector<std::string> children;
  H5Literate(group.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, iter_children, &children);
  return children;
}

void RMGConvertLH5::SetStringAttribute(H5::H5Object& obj, std::string attr_name,
    std::string attr_value) {
  H5::StrType att_dtype(0, H5T_VARIABLE);
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createAttribute(attr_name, att_dtype, scalar);
  att.write(att_dtype, attr_value);
}

std::pair<std::string, std::vector<std::string>> RMGConvertLH5::ReadNullSepDataset(H5::Group& det_group,
    std::string dset_name, std::string ntuple_log_prefix) {
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

H5::DataType RMGConvertLH5::FormToHDFDataType(std::string form_type) {
  if (form_type == "string") return H5::StrType(0, H5T_VARIABLE);
  if (form_type == "int") return H5::PredType::STD_I32LE;
  if (form_type == "float") return H5::PredType::IEEE_F32LE;
  if (form_type == "double") return H5::AtomType(H5::PredType::IEEE_F64LE);
  throw std::logic_error("not implemented form_type, LGDO will be invalid");
}

std::string RMGConvertLH5::DataTypeToLGDO(H5::DataType dtype) {
  auto dtype_class = dtype.getClass();
  switch (dtype_class) {
    case H5T_STRING: return "string";
    case H5T_INTEGER:
    case H5T_FLOAT: return "real";
    default: throw std::logic_error("not implemented H5T type, LGDO will be invalid");
  }
}

bool RMGConvertLH5::ConvertNTupleToTable(H5::Group& det_group) {
  const std::string ntuple_name = det_group.getObjName();
  const std::string ntuple_log_prefix = "ntuple " + ntuple_name + " - ";
  LH5Log(RMGLog::detail, ntuple_log_prefix, "visiting");

  if (!det_group.exists("names") || det_group.childObjType("names") != H5O_TYPE_DATASET ||
      !det_group.exists("forms") || det_group.childObjType("forms") != H5O_TYPE_DATASET ||
      !det_group.exists("columns") || det_group.childObjType("columns") != H5O_TYPE_DATASET) {
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
  uint32_t expected_column_count;
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

  names_string = std::regex_replace(names_string, names_split_re, ",");
  names_string = std::regex_replace(names_string, names_split_re_end, "");
  // set the table lgdo datatype.
  SetStringAttribute(det_group, "datatype", "table{" + names_string + "}");

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
      auto col_idx = std::distance(names_parts.begin(),
          std::find(names_parts.begin(), names_parts.end(), column));
      LH5Log(RMGLog::warning, ntuple_log_prefix, "column ", lgdo_name,
          " - no data, creating with type ", forms_parts[col_idx]);

      hsize_t dset_dataspace_dim[1] = {0};
      H5::DataSpace dset_dataspace(1, dset_dataspace_dim);
      det_group.createDataSet(lgdo_name, FormToHDFDataType(forms_parts[col_idx]), dset_dataspace);
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
  int32_t schema_version;
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
  // warning: this will internally approx. the full file size!
  H5::FileAccPropList fapl;
  if (fDryRun) fapl.setCore(10 * 1024, false);
  else fapl = H5::FileAccPropList::DEFAULT;

  H5::H5File hfile(fHdf5FileName, H5F_ACC_RDWR, H5::FileCreatPropList::DEFAULT, fapl);

  // check that this file has been written by geant4/remage, and that we did not run this upgrade
  // script before (it will delete the header group below).
  if (!hfile.exists("header") || hfile.childObjType("header") != H5O_TYPE_GROUP ||
      !hfile.exists("hit") || hfile.childObjType("hit") != H5O_TYPE_GROUP) {
    LH5Log(RMGLog::error,
        "not a remage HDF5 output file or already converted (missing header or hit groups)?");
    return false;
  }
  auto header_group = hfile.openGroup("header");
  if (!CheckGeantHeader(header_group)) {
    LH5Log(RMGLog::error, "not a remage HDF5 output file (invalid header)?");
    return false;
  }
  LH5Log(RMGLog::detail, "Opened Geant4 HDF5 file ", fHdf5FileName);

  // rework the ntuples to LGDO tables.
  auto hit_group = hfile.openGroup("hit");
  auto ntuples = GetChildren(hit_group);
  bool ntuple_success = true;
  for (auto& ntuple : ntuples) {
    auto det_group = hit_group.openGroup(ntuple);
    ntuple_success &= ConvertNTupleToTable(det_group);
  }

  if (hit_group.attrExists("type")) hit_group.removeAttr("type");

  // check other things that geant4 might write into the file, and delete them if they are empty.
  if (hfile.exists("default_histograms") &&
      hfile.childObjType("default_histograms") == H5O_TYPE_GROUP) {
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

bool RMGConvertLH5::ConvertToLH5(std::string hdf5_file_name, bool dry_run, bool part_of_batch) {
  auto conv = RMGConvertLH5(hdf5_file_name, dry_run, part_of_batch);
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


// vim: tabstop=2 shiftwidth=2 expandtab

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

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
namespace fs = std::filesystem;

#include "RMGLog.hh"

#include "CLI11/CLI11.hpp"
#include "H5Cpp.h"

static std::regex names_split_re("(_in_.+?)?\\0");
static std::regex names_split_re_end("(_in_.+?)$");

int iter_children(long int, const char* name, const H5L_info_t*, void* op_data) {
  auto ntuples = static_cast<std::vector<std::string>*>(op_data);
  ntuples->push_back(name);
  return 0;
}

void set_str_attr(H5::H5Object& obj, std::string attr_name, std::string lgdo_dtype) {
  H5::StrType att_dtype(0, H5T_VARIABLE);
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createAttribute(attr_name, att_dtype, scalar);
  att.write(att_dtype, lgdo_dtype);
}

std::pair<std::string, std::vector<std::string>> read_null_sep_field(H5::Group& det_group,
    std::string dset_name, std::string ntuple_name) {
  // note: the std::string-based API does not read the full string, because geant4 uses the wrong
  // data type (nul-terminated string vs. nul-padded string).
  auto dset = det_group.openDataSet(dset_name);
  size_t storage_size = dset.getStorageSize();
  char* buf = new char[storage_size];
  dset.read(buf, dset.getStrType());

  std::vector<std::string> vec;

  // the last two bytes of the buffer will be NUL.
  if (storage_size < 2 || buf[storage_size - 1] != '\0' || buf[storage_size - 2] != '\0') {
    RMGLog::OutFormat(RMGLog::error, "ntuple {} - invalid {} dataset", ntuple_name, dset_name);
    return {"", vec};
  }
  std::string s(buf, storage_size - 2);

  // split on null bytes.
  std::istringstream is(s);
  std::string tmp;
  while (std::getline(is, tmp, '\0')) vec.push_back(tmp);

  return {s, vec}; // strip the first NUL byte.
}

H5::DataType form_to_hdf5(std::string form_type) {
  if (form_type == "string") return H5::StrType(0, H5T_VARIABLE);
  if (form_type == "int") return H5::PredType::STD_I32LE;
  if (form_type == "float") return H5::PredType::IEEE_F32LE;
  if (form_type == "double") return H5::AtomType(H5::PredType::IEEE_F64LE);
  RMGLog::Out(RMGLog::error, "not implemented form_type, LGDO will be invalid");
  return H5::PredType::IEEE_F64LE;
}

std::string dtype_to_lgdo(H5::DataType dtype) {
  auto dtype_class = dtype.getClass();
  switch (dtype_class) {
    case H5T_STRING: return "string";
    case H5T_INTEGER:
    case H5T_FLOAT: return "real";
    default:
      RMGLog::Out(RMGLog::error, "not implemented H5T type, LGDO will be invalid");
      return "real";
  }
}

void convert_ntuple_to_table(H5::Group& det_group) {
  const std::string ntuple_name = det_group.getObjName();
  RMGLog::OutFormat(RMGLog::detail, "ntuple {} - visiting", ntuple_name);
  if (!det_group.exists("names") || !det_group.exists("forms") || !det_group.exists("columns")) {
    RMGLog::OutFormat(RMGLog::error, "ntuple {} - missing names, forms or columns dataset",
        ntuple_name);
    return;
  }

  // read to check later if we received the correct column count.
  auto dset_columns = det_group.openDataSet("columns");
  if (dset_columns.getDataType() != H5::PredType::STD_U32LE) {
    RMGLog::OutFormat(RMGLog::error, "ntuple {} - invalid columns dataset", ntuple_name);
    return;
  }
  uint32_t expected_column_count;
  dset_columns.read(&expected_column_count, dset_columns.getDataType());

  // read the column names into a buffer. This contains the full data with null bytes.
  auto [names_string, names_parts] = read_null_sep_field(det_group, "names", ntuple_name);
  // "forms" contains the datatypes of Geant.
  auto [forms_string, forms_parts] = read_null_sep_field(det_group, "forms", ntuple_name);

  if (names_string.empty() || forms_string.empty()) return;
  if (names_parts.size() != forms_parts.size()) {
    RMGLog::OutFormat(RMGLog::error, "ntuple {} - mismatch in forms and names count", ntuple_name);
    return;
  }

  names_string = std::regex_replace(names_string, names_split_re, ",");
  names_string = std::regex_replace(names_string, names_split_re_end, "");
  // set the table lgdo datatype.
  set_str_attr(det_group, "datatype", "table{" + names_string + "}");

  // unlink the ntuple definition datasets and attributes.
  det_group.unlink("names");
  det_group.unlink("entries"); // entry count; not relevant here.
  det_group.unlink("forms");
  det_group.unlink("columns");

  det_group.removeAttr("class");
  det_group.removeAttr("type");
  det_group.removeAttr("version");

  // iterate over the remaining child groups and update to LH5 format.
  uint32_t out_column_count = 0;
  std::vector<std::string> columns;
  H5Literate(det_group.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, iter_children, &columns);
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

    RMGLog::OutFormat(RMGLog::detail, "ntuple {} - column {}, with units {}", ntuple_name,
        lgdo_name, lgdo_units);

    // remove the column group with its child dataset, while preserving the data itself.
    det_group.moveLink(column, column + "__tmp");
    if (det_group.nameExists(column + "__tmp/pages")) {
      det_group.moveLink(column + "__tmp/pages", lgdo_name);
    } else {
      // create a new empty dataset, as we have none.
      auto col_idx = std::distance(names_parts.begin(),
          std::find(names_parts.begin(), names_parts.end(), column));
      RMGLog::OutFormat(RMGLog::warning, "ntuple {} - column {} - no data, creating with type {}!",
          ntuple_name, lgdo_name, forms_parts[col_idx]);

      hsize_t dset_dataspace_dim[1] = {0};
      H5::DataSpace dset_dataspace(1, dset_dataspace_dim);
      det_group.createDataSet(lgdo_name, form_to_hdf5(forms_parts[col_idx]), dset_dataspace);
    }
    det_group.unlink(column + "__tmp");

    auto dset_column = det_group.openDataSet(lgdo_name);
    if (unit_sep_pos != std::string::npos) { set_str_attr(dset_column, "units", lgdo_units); }
    // note: this simple boolean decision only works as G4 can only write float/double/int/string.
    // Other types with a distinct lgdo type are not possible to reach here.
    auto lgdo_dtype = dtype_to_lgdo(dset_column.getDataType());
    set_str_attr(dset_column, "datatype", "array<1>{" + lgdo_dtype + "}");
    dset_column.close();

    out_column_count++;
  }

  if (out_column_count != expected_column_count || out_column_count != names_parts.size()) {
    RMGLog::OutFormat(RMGLog::error, "ntuple {} - column count mismatch", ntuple_name);
  }
}

bool check_header(H5::Group header_group) {
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

void convert_remage_to_lh5(std::string file_name) {
  H5::H5File hfile(file_name, H5F_ACC_RDWR);

  // check that this file has been written by geant4/remage, and that we did not run this upgrade
  // script before (it will delete the header group below).
  if (!hfile.exists("header") || !hfile.exists("hit")) {
    RMGLog::Out(RMGLog::error,
        "not a remage HDF5 output file or already converted (missing header or hit groups)?");
    return;
  }
  auto header_group = hfile.openGroup("header");
  if (!check_header(header_group)) {
    RMGLog::Out(RMGLog::error, "not a remage HDF5 output file (invalid header)?");
    return;
  }
  RMGLog::Out(RMGLog::detail, "Opened Geant4 HDF5 file ", file_name);

  // rework the ntuples to LGDO tables.
  auto hit_group = hfile.openGroup("hit");
  std::vector<std::string> ntuples;
  H5Literate(hit_group.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, iter_children, &ntuples);
  for (auto& ntuple : ntuples) {
    auto det_group = hit_group.openGroup(ntuple);
    convert_ntuple_to_table(det_group);
  }

  hit_group.removeAttr("type");

  // check other things that geant4 might write into the file, and delete them if they are empty.
  if (hfile.exists("default_histograms")) {
    std::vector<std::string> histograms;
    H5Literate(hfile.openGroup("default_histograms").getId(), H5_INDEX_NAME, H5_ITER_NATIVE,
        nullptr, iter_children, &histograms);
    if (histograms.empty()) {
      hfile.unlink("default_histograms");
    } else {
      RMGLog::Out(RMGLog::warning, "HDF5 file contains histograms, not yet supported to convert!");
    }
  }

  // remove old header, so that we cannot run this conversion function again.
  hfile.unlink("header");

  hfile.close();

  RMGLog::Out(RMGLog::summary, "Done updating HDF5 file ", file_name, " to LH5");
}

int main(int argc, char** argv) {
  bool verbosity;
  std::vector<std::string> file_names;

  CLI::App app{"remage-to-lh5: convert HDF5 file output files in-place to LH5"};
  app.add_flag("-v", verbosity, "Increase verbosity");
  app.add_option("input_files", file_names, "Input HDF5 files")->type_name("FILE")->required();
  CLI11_PARSE(app, argc, argv);

  RMGLog::SetInihibitStartupInfo(true);
  if (verbosity) RMGLog::SetLogLevel(RMGLog::detail);

  for (auto& file_name : file_names) {
    if (!fs::exists(file_name)) {
      RMGLog::OutFormat(RMGLog::error, "{} does not exist", file_name);
      continue;
    }
    convert_remage_to_lh5(file_name);
  }

  return RMGLog::HadError() ? 1 : 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

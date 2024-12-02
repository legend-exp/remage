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

#include "RMGConvertFromLH5.hh"

#include <cstdint>
#include <regex>
#include <string>
#include <vector>

#include "RMGLog.hh"

int RMGConvertFromLH5::iter_children(long int, const char* name, const H5L_info_t*, void* op_data) {
  auto ntuples = static_cast<std::vector<std::string>*>(op_data);
  ntuples->push_back(name);
  return 0;
}

std::vector<std::string> RMGConvertFromLH5::GetChildren(H5::Group& group) {
  std::vector<std::string> children;
  H5Literate(group.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, iter_children, &children);
  return children;
}

bool RMGConvertFromLH5::ExistsByType(H5::H5Location& loc, std::string name, H5O_type_t type) {
  return loc.nameExists(name) && loc.childObjType(name) == type;
}

void RMGConvertFromLH5::SetStringAttribute(H5::H5Object& obj, std::string attr_name,
    std::string attr_value) {
  H5::StrType att_dtype(0, attr_value.size() + 1);
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createAttribute(attr_name, att_dtype, scalar);
  att.write(att_dtype, attr_value);
}

std::optional<std::string> RMGConvertFromLH5::GetStringAttribute(H5::H5Object& obj,
    std::string attr_name) {
  if (!obj.attrExists("datatype")) return std::nullopt;
  auto att_writer = obj.openAttribute(attr_name);
  if (att_writer.getDataType().getClass() != H5T_STRING) return std::nullopt;
  std::string writer;
  att_writer.read(att_writer.getDataType(), writer);
  return writer;
}

void RMGConvertFromLH5::CreateUIntDataset(H5::Group& obj, std::string dset_name, uint64_t attr_value) {
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createDataSet(dset_name, H5::PredType::STD_I64LE, scalar);
  att.write(&attr_value, H5::PredType::STD_I32LE);
}

void RMGConvertFromLH5::CreateStringDataset(H5::Group& obj, std::string dset_name,
    std::string attr_value) {
  H5::StrType att_dtype(0, attr_value.size() + 1);
  H5::DataSpace scalar(H5S_SCALAR);
  auto att = obj.createDataSet(dset_name, att_dtype, scalar);
  att.write(attr_value, att_dtype);
}

std::string RMGConvertFromLH5::HDFDataTypeToForm(H5::DataType dtype) {
  auto dtype_class = dtype.getClass();
  switch (dtype_class) {
    case H5T_STRING: return "string";
    case H5T_INTEGER: return "int";
    case H5T_FLOAT: return dtype.getSize() == 8 ? "double" : "float";
    default: throw std::logic_error("not implemented H5T type, LGDO will be invalid");
  }
}

bool RMGConvertFromLH5::ConvertTableToNTuple(H5::Group& det_group) {
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

  if (datatype.empty()) return false;

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
  int out_entries_count = -1;
  for (auto& lgdo_name : columns) {
    auto column = lgdo_name;

    auto dset_column = det_group.openDataSet(lgdo_name);
    if (det_group.attrExists("units")) {
      column += "_in_" + GetStringAttribute(det_group, "units").value();
    }
    names_string += column + std::string("\0", 1);
    forms_string += HDFDataTypeToForm(dset_column.getDataType()) + std::string("\0", 1);
    dset_column.close();

    LH5Log(RMGLog::detail, ntuple_log_prefix, "column ", lgdo_name, " to ", column);

    // move the column to a pages array.
    std::string column_tmp = column + "__tmp";
    det_group.moveLink(column, column_tmp);
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
    if (out_entries_count < 0) {
      out_entries_count = dims[0];
    } else if (out_entries_count != dims[0]) {
      LH5Log(RMGLog::error, ntuple_log_prefix, "mismatch for entry count for column ", column, " ",
          out_entries_count, " vs ", dims[0]);
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

bool RMGConvertFromLH5::ConvertFromLH5Internal() {
  // using the core driver with no backing storage will allow to change the file purely in-memory.
  // warning: this will internally approx. the full file size!
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
    ntuple_success &= ConvertTableToNTuple(det_group);
  }

  if (ntuples_group.attrExists("datatype")) ntuples_group.removeAttr("datatype");

  if (ntuple_group_name != "default_ntuples") {
    hfile.moveLink(ntuple_group_name, "default_ntuples");
  }

  hfile.close();

  LH5Log(RMGLog::summary, "Done updating HDF5 file ", fHdf5FileName, " to LH5");

  return ntuple_success;
}

bool RMGConvertFromLH5::ConvertFromLH5(std::string lh5_file_name, std::string ntuple_group_name,
    bool dry_run, bool part_of_batch) {
  auto conv = RMGConvertFromLH5(lh5_file_name, ntuple_group_name, dry_run, part_of_batch);
  try {
    return conv.ConvertFromLH5Internal();
  } catch (const H5::Exception& e) {
    conv.LH5Log(RMGLog::error, e.getDetailMsg());
    return false;
  } catch (const std::logic_error& e) {
    conv.LH5Log(RMGLog::error, e.what());
    return false;
  }
}


// vim: tabstop=2 shiftwidth=2 expandtab

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

#include <iostream>
#include <string>

#include "H5Cpp.h"

static bool dump_lh5_datatype = false;

int visit(H5::H5Object& obj, const std::string name, const H5O_info_t*, void*) {
  if (name == ".") return 0;

  auto dump = name;
  // note that there is apparently no C++ API to get a generic H%::Object by name... :-(
  hid_t o = H5Oopen(obj.getId(), name.c_str(), H5P_DEFAULT);
  if (dump_lh5_datatype) {
    if (H5Aexists(o, "datatype")) {
      auto att_dtype = H5Aopen(o, "datatype", H5P_DEFAULT);
      auto att_dtype_dtype = H5Aget_type(att_dtype);
      if (H5Tget_class(att_dtype_dtype) == H5T_STRING) {
        char* datatype = new char[H5Tget_size(att_dtype_dtype)];
        H5Aread(att_dtype, att_dtype_dtype, &datatype);
        dump += std::string(" : ") + datatype;
      }
      H5Tclose(att_dtype_dtype);
      H5Aclose(att_dtype);
    }
  }
  H5Oclose(o);

  std::cout << dump << std::endl;
  return 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "input filename missing" << std::endl;
    return 1;
  }
  if (argc > 2 && std::string(argv[2]) == "--dump-attrs") dump_lh5_datatype = true;

  H5::H5File hfile(argv[1], H5F_ACC_RDONLY);
  hfile.visit(H5_INDEX_NAME, H5_ITER_INC, visit, nullptr, 0);
  hfile.close();

  return 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

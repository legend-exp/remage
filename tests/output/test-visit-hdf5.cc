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

int visit(H5::H5Object&, const std::string name, const H5O_info_t*, void*) {
  if (name != ".") std::cout << name << std::endl;
  return 0;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "input filename missing" << std::endl;
    return 1;
  }

  H5::H5File hfile(argv[1], H5F_ACC_RDONLY);
  hfile.visit(H5_INDEX_NAME, H5_ITER_INC, visit, nullptr, 0);
  hfile.close();

  return 0;
}

// vim: tabstop=2 shiftwidth=2 expandtab

// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "G4UImanager.hh"

#include "RMGGeneratorCosmicMuons.hh"
#include "RMGGeneratorMUSUNCosmicMuons.hh"
#include "RMGGermaniumOutputScheme.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOpticalOutputScheme.hh"
#include "RMGScintillatorOutputScheme.hh"
#include "RMGVOutputScheme.hh"
#include "RMGVertexConfinement.hh"
#include "RMGVertexFromFile.hh"
#include "RMGVertexOutputScheme.hh"

void init_extra() {
  // initialize non-default things that have messengers.
  // add here other things in the future.

  // output schemes
  new RMGGermaniumOutputScheme();
  new RMGOpticalOutputScheme();
  new RMGVertexOutputScheme();
  new RMGScintillatorOutputScheme();
  // generators
  new RMGGeneratorMUSUNCosmicMuons();
  new RMGGeneratorCosmicMuons();
  // confinments
  new RMGVertexConfinement();
  new RMGVertexFromFile();
}

int main(int argc, char** argv) {
  std::string dir = ".";
  if (argc > 1) dir = argv[1];

  RMGLog::SetLogLevel(RMGLog::summary);

  RMGManager manager("remage-doc-dump", argc, argv);
  manager.Initialize();

  init_extra();

  if (dir != ".") {
    RMGLog::Out(RMGLog::summary, "Set output directory to ", dir);
    auto path = std::filesystem::path(dir);
    std::filesystem::create_directories(path);
    std::filesystem::current_path(path);
  }

  auto UI = G4UImanager::GetUIpointer();
  RMGLog::Out(RMGLog::summary, "Export HTML");
  UI->ApplyCommand("/control/createHTML /RMG/");

  RMGLog::Out(RMGLog::summary, "Export TXT");
  std::ofstream out("rmg-manual.txt");
  std::streambuf* coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());
  UI->ApplyCommand("/control/manual /RMG/");
  std::cout.rdbuf(coutbuf);
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#include "G4ApplicationState.hh"
#include "G4UIcommandTree.hh"
#include "G4UImanager.hh"

#include "RMGGeneratorCosmicMuons.hh"
#include "RMGGeneratorFromFile.hh"
#include "RMGGeneratorMUSUNCosmicMuons.hh"
#include "RMGGermaniumOutputScheme.hh"
#include "RMGIsotopeFilterOutputScheme.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOpticalOutputScheme.hh"
#include "RMGParticleFilterOutputScheme.hh"
#include "RMGScintillatorOutputScheme.hh"
#include "RMGTrackOutputScheme.hh"
#include "RMGVOutputScheme.hh"
#include "RMGVertexConfinement.hh"
#include "RMGVertexFromFile.hh"
#include "RMGVertexOutputScheme.hh"

#include "CLI/CLI.hpp"
#include "magic_enum/magic_enum.hpp"

void init_extra() {
  // initialize non-default things that have messengers.
  // add here other things in the future.

  // output schemes
  new RMGGermaniumOutputScheme();
  new RMGOpticalOutputScheme();
  new RMGVertexOutputScheme();
  new RMGScintillatorOutputScheme();
  new RMGIsotopeFilterOutputScheme();
  new RMGTrackOutputScheme();
  new RMGParticleFilterOutputScheme();
  // generators
  new RMGGeneratorMUSUNCosmicMuons();
  new RMGGeneratorCosmicMuons();
  new RMGGeneratorFromFile();
  // confinments
  new RMGVertexConfinement();
  new RMGVertexFromFile();
}

void list_tree(G4UIcommandTree* tree);

int main(int argc, char** argv) {
  CLI::App app{"remage-doc-dump"};
  std::string html_dir, manual_file;
  app.add_option("--html", html_dir, "GDML files")->type_name("FILE");
  app.add_option("--manual", manual_file, "Output file for detector hits")->type_name("FILE");
  CLI11_PARSE(app, argc, argv);

  RMGLog::SetLogLevel(RMGLog::summary);

  RMGManager manager("remage-doc-dump", argc, argv);
  manager.Initialize();

  init_extra();

  auto ui = G4UImanager::GetUIpointer();
  auto rmg_tree = ui->GetTree()->FindCommandTree("/RMG/");
  if (!rmg_tree) {}

  if (!html_dir.empty()) {
    RMGLog::Out(RMGLog::summary, "Export HTML to ", html_dir);
    auto path = std::filesystem::path(html_dir);
    std::filesystem::create_directories(path);
    std::filesystem::current_path(path);

    rmg_tree->CreateHTML();
  }

  if (!manual_file.empty()) {
    RMGLog::Out(RMGLog::summary, "Export TXT to ", manual_file);
    std::ofstream out(manual_file);
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());
    list_tree(rmg_tree);
    std::cout.rdbuf(coutbuf);
  }
}

void list_tree(G4UIcommandTree* tree) {
  // List summary.
  tree->ListCurrent();

  // List commands.
  auto command_count = tree->GetCommandEntry();
  for (auto i = 1; i <= command_count; i++) {
    auto cmd = tree->GetCommand(i);
    cmd->List();

    // add an additional line about allowed application states.
    std::string states;
    auto state_regex = std::regex("^G4State_");
    for (const auto& state : (*cmd->GetStateList())) {
      auto name = std::string(magic_enum::enum_name<G4ApplicationState>(state));
      name = std::regex_replace(name, state_regex, "");
      states += name + " ";
    }
    states = states.substr(0, states.size() - 1);

    G4cout << "Allowed states : " << states << G4endl;
  }

  // (Recursively) list subdirectories.
  auto tree_count = tree->GetTreeEntry();
  for (auto i = 1; i <= tree_count; i++) { list_tree(tree->GetTree(i)); }
}

// vim: tabstop=2 shiftwidth=2 expandtab

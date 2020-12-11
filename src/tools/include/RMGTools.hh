#include <memory>
#include <vector>
#include <utility>
#include <ctime>
#include <chrono>

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4ApplicationState.hh"

namespace RMGTools {

  template <typename T> // G4UIcmdWithA[...]AndUnit
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg,
      std::vector<G4ApplicationState> avail_for, G4String unit_cat, G4String unit_cand="",
      G4String par_name="", G4String range="");

  template <typename T> // G4UIcmdWithA[...]
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg,
      std::vector<G4ApplicationState> avail_for, G4String par_name="", G4String range="");

  template <typename T> // G4cmdWithAString
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg,
      std::vector<G4ApplicationState> avail_for, G4String candidates);

  template <typename T> // G4cmdWithABool
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg,
      std::vector<G4ApplicationState> avail_for, G4bool default_val, G4bool omittable=false);

  std::tm ToUTCTime(std::chrono::time_point<std::chrono::system_clock> t);
}

// vim: shiftwidth=2 tabstop=2 expandtab

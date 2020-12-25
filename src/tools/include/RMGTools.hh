#ifndef _RMG_TOOLS_HH_
#define _RMG_TOOLS_HH_

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
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg, G4String unit_cat,
      G4String unit_cand="", G4String par_name="", G4String range="",
      std::vector<G4ApplicationState> avail_for={G4State_Init, G4State_PreInit});

  template <typename T> // G4UIcmdWithA[...]
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg, G4String par_name="",
      G4String range="", std::vector<G4ApplicationState> avail_for={G4State_Init, G4State_PreInit});

  template <typename T> // G4cmdWithAString
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg, G4String candidates,
      std::vector<G4ApplicationState> avail_for={G4State_Init, G4State_PreInit});

  template <typename T> // G4cmdWithABool
  std::unique_ptr<T> MakeG4UIcmd(G4String name, G4UImessenger* msg, G4bool default_val,
      G4bool omittable=false, std::vector<G4ApplicationState> avail_for={G4State_Init, G4State_PreInit});

  std::tm ToUTCTime(std::chrono::time_point<std::chrono::system_clock> t);
}

#include "RMGMessengerTools.icc"

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

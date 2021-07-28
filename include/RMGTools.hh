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
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

namespace RMGTools {

  std::tm ToUTCTime(std::chrono::time_point<std::chrono::system_clock> t);

  template <typename T>
  T ToEnum(G4String);

  template <class T> // G4UIcmdWithA[...]
  std::unique_ptr<T> MakeG4UIcmdWithANumber(G4String name, G4UImessenger* msg, G4String par_name="",
      G4String range="", std::vector<G4ApplicationState> avail_for={G4State_Init});

  template <class T> // G4UIcmdWithA[...]AndUnit
  std::unique_ptr<T> MakeG4UIcmdWithANumberAndUnit(G4String name, G4UImessenger* msg,
      G4String unit_cat, G4String unit_cand="", G4String par_name="", G4String range="",
      std::vector<G4ApplicationState> avail_for={G4State_Init});

  std::unique_ptr<G4UIcmdWith3Vector> MakeG4UIcmdWith3Vector(G4String name, G4UImessenger* msg,
      std::vector<G4String> par_name={"", "", ""}, G4String range="",
      std::vector<G4ApplicationState> avail_for={G4State_Init});

  std::unique_ptr<G4UIcmdWith3VectorAndUnit> MakeG4UIcmdWith3VectorAndUnit(G4String name,
      G4UImessenger* msg, G4String unit_cat, G4String unit_cand="",
      std::vector<G4String> par_name={"", "", ""}, G4String range="",
      std::vector<G4ApplicationState> avail_for={G4State_Init});

  std::unique_ptr<G4UIcmdWithAString> MakeG4UIcmdWithAString(G4String name, G4UImessenger* msg,
      G4String candidates="", std::vector<G4ApplicationState> avail_for={G4State_Init});

  std::unique_ptr<G4UIcmdWithABool> MakeG4UIcmdWithABool(G4String name, G4UImessenger* msg,
      G4bool omittable=false, std::vector<G4ApplicationState> avail_for={G4State_Init});
}

#include "RMGMessengerTools.icc"
#include "RMGTools.icc"

#endif

// vim: shiftwidth=2 tabstop=2 expandtab

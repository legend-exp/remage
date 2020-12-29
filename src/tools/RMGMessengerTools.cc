#include "RMGTools.hh"

#include "RMGLog.hh"

std::unique_ptr<G4UIcmdWithAString> RMGTools::MakeG4UIcmdWithAString(G4String name,
    G4UImessenger* msg, G4String candidates,
    std::vector<G4ApplicationState> avail_for) {

  std::unique_ptr<G4UIcmdWithAString> cmd(new G4UIcmdWithAString(name.c_str(), msg));
  for (auto& s : avail_for) cmd->AvailableForStates(s);
  if (!candidates.empty()) cmd->SetCandidates(candidates);

  return cmd;
}

std::unique_ptr<G4UIcmdWithABool> RMGTools::MakeG4UIcmdWithABool(G4String
    name, G4UImessenger* msg, G4bool omittable,
    std::vector<G4ApplicationState> avail_for) {

  std::unique_ptr<G4UIcmdWithABool> cmd(new G4UIcmdWithABool(name.c_str(), msg));
  for (auto& s : avail_for) cmd->AvailableForStates(s);
  cmd->SetParameterName("", omittable);

  return cmd;
}

std::unique_ptr<G4UIcmdWith3Vector> RMGTools::MakeG4UIcmdWith3Vector(G4String name,
    G4UImessenger* msg, std::vector<G4String> par_name,
    G4String range, std::vector<G4ApplicationState> avail_for) {

  if (par_name.size() != 3) {
      RMGLog::Out(RMGLog::fatal, "Parameter name vector must have size = 3");
  }

  std::unique_ptr<G4UIcmdWith3Vector> cmd(new G4UIcmdWith3Vector(name.c_str(), msg));
  for (auto& s : avail_for) cmd->AvailableForStates(s);
  cmd->SetParameterName(par_name[0], par_name[1], par_name[2], false);
  if (!range.empty()) cmd->SetRange(range);

  return cmd;
}

std::unique_ptr<G4UIcmdWith3VectorAndUnit> RMGTools::MakeG4UIcmdWith3VectorAndUnit(G4String name,
    G4UImessenger* msg, G4String unit_cat, G4String unit_cand, std::vector<G4String> par_name,
    G4String range, std::vector<G4ApplicationState> avail_for) {

  if (par_name.size() != 3) {
      RMGLog::Out(RMGLog::fatal, "Parameter name vector must have size = 3");
  }

  std::unique_ptr<G4UIcmdWith3VectorAndUnit> cmd(new G4UIcmdWith3VectorAndUnit(name.c_str(), msg));
  for (auto& s : avail_for) cmd->AvailableForStates(s);
  cmd->SetParameterName(par_name[0], par_name[1], par_name[2], false);
  cmd->SetUnitCategory(unit_cat);
  if (!unit_cand.empty()) cmd->SetUnitCandidates(unit_cand);
  if (!range.empty()) cmd->SetRange(range);

  return cmd;
}

// vim: shiftwidth=2 tabstop=2 expandtab

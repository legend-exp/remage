#include "RMGNavigationTools.hh"

#include <set>
#include <map>

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4Material.hh"

#include "RMGLog.hh"

#include "fmt/core.h"

G4VPhysicalVolume* RMGNavigationTools::FindVolumeByName(G4String name) {
    auto& store = *G4PhysicalVolumeStore::GetInstance();
    auto result = std::find_if(store.begin(), store.end(), [&name](auto v) { return std::string(v->GetName()) == name; });
    if (result == store.end()) {
      RMGLog::Out(RMGLog::error, "Volume ", name, " not found in volume store. Returning nullptr");
      return nullptr;
    }
    return *result;
}

G4VPhysicalVolume* RMGNavigationTools::FindDirectMother(G4VPhysicalVolume* volume) {

  std::set<G4VPhysicalVolume*> ancestors;
  for (const auto& v : *G4PhysicalVolumeStore::GetInstance()) {
    if (v->GetLogicalVolume()->IsAncestor(volume)) ancestors.insert(v);
  }

  if (ancestors.empty()) {
    RMGLog::Out(RMGLog::error, "No ancestors found for physical volume '",
        volume->GetName(), "', returning nullptr");
  }

  // check elements one-by-one
  for (auto it = ancestors.begin(); it != ancestors.end(); ) {
    // determine if element should be erased or not because it is an ancestor
    // of someone else in the list
    G4bool to_erase = false;
    for (auto itt = ancestors.begin(); itt != ancestors.end(); ) {
      if (*it != *itt and (*it)->GetLogicalVolume()->IsAncestor(*itt)) {
        to_erase = true;
        break;
      }
      else itt++;
    }
    if (to_erase) it = ancestors.erase(it);
    else it++;
  }

  if (ancestors.size() != 1) {
    RMGLog::Out(RMGLog::error, "Could not find a unique direct mother volume, ",
        "this cannot be! Returning first ancestor in the list");
  }

  return *ancestors.begin();
}

void RMGNavigationTools::PrintListOfLogicalVolumes() {

  G4int max_length = 0;
  // use std::set to have volumes automatically sorted by name
  std::set<std::pair<G4String, G4String>> volumes;
  for (const auto& v : *G4LogicalVolumeStore::GetInstance()) {

    if (v->GetName().size() > std::size_t(max_length)) max_length = v->GetName().size();

    volumes.insert({v->GetName(),
        fmt::format("{} daugh. // {} // {} // {} // {} // {}",
            v->GetNoDaughters(),
            G4String(G4BestUnit(v->GetMaterial()->GetDensity(), "Volumic Mass")),
            G4String(G4BestUnit(v->GetMass(), "Mass")),
            G4String(G4BestUnit(v->GetMass() / v->GetMaterial()->GetDensity(), "Volume")),
            G4String(G4BestUnit(v->GetMaterial()->GetPressure(), "Pressure")),
            G4String(G4BestUnit(v->GetMaterial()->GetTemperature(), "Temperature")))
        });
  }

  RMGLog::Out(RMGLog::summary, "Logical volumes registered in the volume store (alphabetic order):");
  for (const auto& v : volumes) {
    RMGLog::OutFormat(RMGLog::summary, " · {:<" + std::to_string(max_length) + "} // {}", v.first, v.second);
  }
  RMGLog::Out(RMGLog::summary, "");
  RMGLog::Out(RMGLog::summary, "Total: ", volumes.size(), " volumes");
}

void RMGNavigationTools::PrintListOfPhysicalVolumes() {

  std::vector<G4String> volumes;
  for (const auto& v : *G4PhysicalVolumeStore::GetInstance()) {
    volumes.push_back(" · " + v->GetName() + " (" + std::to_string(v->GetCopyNo()) +
        + ") // from logical: " + v->GetLogicalVolume()->GetName());
  }
  std::sort(volumes.begin(), volumes.end());

  RMGLog::Out(RMGLog::summary, "Physical volumes registered in the volume store in alphabetic order [name (copy nr.)]:");
  for (const auto& v : volumes) RMGLog::Out(RMGLog::summary, v);
  RMGLog::Out(RMGLog::summary, "");
  RMGLog::Out(RMGLog::summary, "Total: ", volumes.size(), " volumes");
}

// vim: tabstop=2 shiftwidth=2 expandtab

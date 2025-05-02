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

#include "RMGNavigationTools.hh"

#include <map>
#include <set>

#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4UnitsTable.hh"

#include "RMGLog.hh"

#include "fmt/core.h"

G4VPhysicalVolume* RMGNavigationTools::FindPhysicalVolume(std::string name, int copy_nr) {
  auto const& store = *G4PhysicalVolumeStore::GetInstance();
  auto result = std::find_if(store.begin(), store.end(), [&name, &copy_nr](auto v) {
    return (std::string(v->GetName()) == name && v->GetCopyNo() == copy_nr);
  });
  if (result == store.end()) {
    RMGLog::Out(RMGLog::error, "Physical volume ", name, " not found in store. Returning nullptr");
    return nullptr;
  }
  return *result;
}

G4LogicalVolume* RMGNavigationTools::FindLogicalVolume(std::string name) {
  auto const& store = *G4LogicalVolumeStore::GetInstance();
  auto result = std::find_if(store.begin(), store.end(), [&name](auto v) {
    return std::string(v->GetName()) == name;
  });
  if (result == store.end()) {
    RMGLog::Out(RMGLog::error, "Logical volume ", name, " not found in store. Returning nullptr");
    return nullptr;
  }
  return *result;
}

G4VPhysicalVolume* RMGNavigationTools::FindDirectMother(G4VPhysicalVolume* volume) {

  auto ancestors = RMGNavigationTools::FindDirectMothers(volume);

  if (ancestors.size() != 1) {
    RMGLog::Out(
        RMGLog::fatal,
        "Could not find a unique direct mother volume, ",
        "this cannot be! Returning first ancestor in the list"
    );
  }

  return *ancestors.begin();
}

std::set<G4VPhysicalVolume*> RMGNavigationTools::FindDirectMothers(G4VPhysicalVolume* volume) {

  std::set<G4VPhysicalVolume*> ancestors;
  for (const auto& v : *G4PhysicalVolumeStore::GetInstance()) {
    if (v->GetLogicalVolume()->IsAncestor(volume)) ancestors.insert(v);
  }

  if (ancestors.empty()) {
    RMGLog::Out(
        RMGLog::error,
        "No ancestors found for physical volume '",
        volume->GetName(),
        "', returning nullptr"
    );
  }

  // check elements one-by-one
  for (auto it = ancestors.begin(); it != ancestors.end();) {
    // determine if element should be erased or not because it is an ancestor
    // of someone else in the list
    bool to_erase = false;
    for (auto itt = ancestors.begin(); itt != ancestors.end();) {
      if (*it != *itt and (*it)->GetLogicalVolume()->IsAncestor(*itt)) {
        to_erase = true;
        break;
      } else itt++;
    }
    if (to_erase) it = ancestors.erase(it);
    else it++;
  }

  return ancestors;
}

void RMGNavigationTools::PrintListOfLogicalVolumes() {

  size_t max_length = 0;
  // use std::set to have volumes automatically sorted by name
  std::set<std::pair<std::string, std::string>> volumes;
  for (const auto& v : *G4LogicalVolumeStore::GetInstance()) {

    if (v->GetName().size() > max_length) max_length = v->GetName().size();

    volumes.insert(
        {v->GetName(),
         fmt::format(
             "{} daugh. // {} // {} // {} // {} // {}",
             v->GetNoDaughters(),
             std::string(G4BestUnit(v->GetMaterial()->GetDensity(), "Volumic Mass")),
             std::string(G4BestUnit(v->GetMass(), "Mass")),
             std::string(G4BestUnit(v->GetMass() / v->GetMaterial()->GetDensity(), "Volume")),
             std::string(G4BestUnit(v->GetMaterial()->GetPressure(), "Pressure")),
             std::string(G4BestUnit(v->GetMaterial()->GetTemperature(), "Temperature"))
         )}
    );
  }

  RMGLog::Out(RMGLog::summary, "Logical volumes registered in the volume store (alphabetic order):");
  for (const auto& v : volumes) {
    RMGLog::OutFormat(
        RMGLog::summary,
        " · {:<" + std::to_string(max_length) + "} // {}",
        v.first,
        v.second
    );
  }
  RMGLog::Out(RMGLog::summary, "");
  RMGLog::Out(RMGLog::summary, "Total: ", volumes.size(), " volumes");
}

void RMGNavigationTools::PrintListOfPhysicalVolumes() {

  std::vector<std::string> volumes;
  for (const auto& v : *G4PhysicalVolumeStore::GetInstance()) {
    volumes.push_back(
        " · " + v->GetName() + " (" + std::to_string(v->GetCopyNo()) +
        +") // from logical: " + v->GetLogicalVolume()->GetName()
    );
  }
  std::sort(volumes.begin(), volumes.end());

  RMGLog::Out(
      RMGLog::summary,
      "Physical volumes registered in the volume store in alphabetic order [name (copy nr.)]:"
  );
  for (const auto& v : volumes) RMGLog::Out(RMGLog::summary, v);
  RMGLog::Out(RMGLog::summary, "");
  RMGLog::Out(RMGLog::summary, "Total: ", volumes.size(), " volumes");
}

// vim: tabstop=2 shiftwidth=2 expandtab

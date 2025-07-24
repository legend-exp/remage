// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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
#include <queue>
#include <set>

#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4TransportationManager.hh"
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


std::vector<RMGNavigationTools::VolumeTreeEntry> RMGNavigationTools::FindGlobalPositions(
    G4VPhysicalVolume* pv
) {
  auto world_volume = G4TransportationManager::GetTransportationManager()
                          ->GetNavigatorForTracking()
                          ->GetWorldVolume();

  std::vector<VolumeTreeEntry> trees;
  // queue for paths to the mother volume that still have to be searched.
  std::queue<VolumeTreeEntry> q;
  q.emplace(pv);

  for (; !q.empty(); q.pop()) {
    auto v = q.front();

    if (!v.physvol)
      RMGLog::OutDev(
          RMGLog::fatal,
          "nullptr detected in loop condition, this is unexpected. ",
          "Blame RMGNavigationTools::FindDirectMother?"
      );

    v.partial_rotations.push_back(v.physvol->GetObjectRotationValue());
    v.partial_translations.push_back(v.physvol->GetObjectTranslation());

    v.vol_global_rotation = v.partial_rotations.back() * v.vol_global_rotation;

    for (auto m : RMGNavigationTools::FindDirectMothers(v.physvol)) {
      if (m != world_volume) {
        auto v_m = VolumeTreeEntry(v); // create a copy of the current helper object.
        v_m.physvol = m;
        q.push(v_m);
      } else { // we finished that branch!
        trees.push_back(v);
      }
    }
  }

  RMGLog::OutFormatDev(
      RMGLog::debug,
      "Found {} ways to reach world volume from {}",
      trees.size(),
      pv->GetName()
  );

  // finalize all found paths to the mother volume.
  for (auto&& v : trees) {
    // world volume not included in loop
    v.partial_translations.emplace_back(); // origin
    v.partial_rotations.emplace_back();    // identity

    // partial_rotations[0] and partial_translations[0] refer to the target
    // volume partial_rotations[1] and partial_translations[1], to the direct
    // mother, etc. It is necessary to rotate with respect to the frame of the
    // mother. If there are no rotations (or only the target volume is
    // rotated): rotations are identity matrices and vol_global_translation =
    // sum(partial_translations)
    for (size_t i = 0; i < v.partial_translations.size() - 1; i++) {
      G4ThreeVector tmp = v.partial_translations[i];
      for (size_t j = i + 1; j < v.partial_rotations.size() - 1; j++) {
        tmp *= v.partial_rotations[j];
      }
      v.vol_global_translation += tmp;
    }
  }

  if (trees.empty())
    RMGLog::OutDev(RMGLog::fatal, "No path to world volume found, that should not be!");

  return trees;
}

// vim: tabstop=2 shiftwidth=2 expandtab

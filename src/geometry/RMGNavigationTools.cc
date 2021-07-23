#include "RMGNavigationTools.hh"

#include <set>
#include <vector>

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolume.hh"

#include "RMGLog.hh"

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

void RMGNavigationTools::PrintListOfPhysicalVolumes() {

  std::vector<G4String> volumes;
  for (const auto& v : *G4PhysicalVolumeStore::GetInstance()) {
    volumes.push_back(v->GetName() + ", copy nr. " + G4String(v->GetCopyNo()));
  }
  std::sort(volumes.begin(), volumes.end());

  RMGLog::Out(RMGLog::summary, "Physical volumes registered in the volume store:");
  RMGLog::Out(RMGLog::summary, "------------------------------------------------");
  for (const auto& v : volumes) RMGLog::Out(RMGLog::summary, v);
  RMGLog::Out(RMGLog::summary, "------------------------------------------------");
  RMGLog::Out(RMGLog::summary, "Total: ", volumes.size(), " volumes");
}

// vim: tabstop=2 shiftwidth=2 expandtab

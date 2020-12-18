#include "RMGMaterialTable.hh"

#include "RMGMaterialTableMessenger.hh"

RMGMaterialTable::RMGMaterialTable() {
  fG4Messenger = std::unique_ptr<RMGMaterialTableMessenger>(new RMGMaterialTableMessenger(this));

  this->InitializeMaterials();
  this->InitializeOpticalProperties();
}

void RMGMaterialTable::InitializeMaterials() {
}

void RMGMaterialTable::InitializeOpticalProperties() {
}

// vim: tabstop=2 shiftwidth=2 expandtab

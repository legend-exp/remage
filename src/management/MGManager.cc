#include "MGManager.hh"

#include "G4RunManager.hh"
#include "G4VisManager.hh"

#include "MGLog.hh"
#include "MGManagerMessenger.hh"

MGManager* MGManager::fMGManager = 0;

MGManager::MGManager() {
  if (fMGManager) MGLog::OutFatal("MGManager must be singleton!");
  fMGManager = this;
  fG4Messenger = new MGManagerMessenger(this);
}

MGManager::~MGManager() {
  delete fG4Messenger;
}

// vim: tabstop=2 shiftwidth=2 expandtab

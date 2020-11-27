#include "MGGeneratorPrimary.hh"

#include <string>
#include <sstream>

#include "CLHEP/Units/SystemOfUnits.h"
#include "Randomize.hh"
#include "G4RandomDirection.hh"
#include "G4PhysicalVolumeStore.hh"

#include "MGGeneratorPrimaryMessenger.hh"
#include "MGGeneratorPositionSampling.hh"
#include "MGVGenerator.hh"
#include "MGLog.hh"

MGGeneratorPrimary::MGGeneratorPrimary():
  fConfinementCode(EConfinementCode::noconfined),
  fPosition(0, 0, 0),
  fVolumeName("nullptr"),
  fVolumeListName("nullptr"),
  fVolumeListFrom(0),
  fVolumeListTo(0),
  fVolumeListInitialized(false),
  fVolumeArrayInitialized(false) {

  fPositionSampler = new MGGeneratorPositionSampling();
  fG4Messenger = new MGGeneratorPrimaryMessenger(this);
}

MGGeneratorPrimary::~MGGeneratorPrimary() {
  delete fG4Messenger;
  delete fPositionSampler;
  delete fMGGenerator;
}

void MGGeneratorPrimary::GeneratePrimaries(G4Event* event) {
  G4int copy_nr = 0;
  if (fMGGenerator) {
    if (fConfinementCode == volume) {
      fPosition = fPositionSampler->SampleUniformlyInVolume(GetVolumeName(), copy_nr);
      fMGGenerator->SetParticlePosition(fPosition);
    }
    else if (fConfinementCode == volumelist) {
      if (this->GetVolumeListName() == "nullptr") {
        MGLog::Out(MGLog::fatal, "Volume name list is not defined, please set it using the proper macro command");
      }

      G4String volume_name = this->ChooseVolumeFromList();
      MGLog::Out(MGLog::debug, "Volume to be sampled from: ", volume_name);

      fPosition = fPositionSampler->SampleUniformlyInVolume(volume_name, copy_nr);
      fMGGenerator->SetParticlePosition(fPosition);
    }
    else if (fConfinementCode == volumearray) {
      G4String volume_name = ChooseVolumeFromArray();
      MGLog::Out(MGLog::debug, "Volume to be sampled from: ", volume_name);

      fPosition = fPositionSampler->SampleUniformlyInVolume(volume_name, copy_nr);
      fMGGenerator->SetParticlePosition(fPosition);
    }

    fMGGenerator->GeneratePrimaryVertex(event);
  }
  else MGLog::Out(MGLog::fatal, "No generator specified!");
}

void MGGeneratorPrimary::SetConfinementCode(EConfinementCode code) {

  fConfinementCode = code;
  if (fConfinementCode == noconfined) {
    // reset the starting position
    fPosition = G4ThreeVector(0, 0, 0);
    fMGGenerator->SetParticlePosition(fPosition);
    MGLog::Out(MGLog::detail, "Default position re-set to ", fPosition);
  }
}

void MGGeneratorPrimary::InitializeVolumeArraySampling() {

  MGLog::Out(MGLog::summary, "Start InitializeVolumeArraySampling");

  fMassFractionForVolumeArray.clear();
  // fIDVolumeArray.clear();

  G4double tot_mass = 0;
  G4String volume_name;
  auto phys_vol_store = G4PhysicalVolumeStore::GetInstance();

  // First element is zero
  fMassFractionForVolumeArray.push_back(tot_mass);

  if (fIDVolumeArray.size() == 0) {
    MGLog::Out(MGLog::error, "Problem with the volumearray: no list of volumes selected ",
        "(neither one by one nor list), Changing confining code to 'unconfined'");
    fConfinementCode = noconfined;
  }

  // Get masses of volumes involved
  for (size_t i = 0; i < fIDVolumeArray.size(); i++) {
    volume_name = fIDVolumeArray.at(i);
    auto phys_vol = phys_vol_store->GetVolume(volume_name);
    if (!phys_vol) {
      MGLog::OutFormat(MGLog::error, "The '%s' volume does not exist", volume_name.c_str());
      return;
    }

    G4double vol_mass = phys_vol->GetLogicalVolume()->GetMass(false, false);
    MGLog::OutFormat(MGLog::summary, "Mass of volume '%s' is %g", volume_name.c_str(), vol_mass/CLHEP::kg);
    tot_mass += vol_mass;
    fMassFractionForVolumeArray.push_back(tot_mass);
  }

  MGLog::Out(MGLog::summary, "Number of volumes: ", fMassFractionForVolumeArray.size()-1);
  MGLog::Out(MGLog::summary, "Total mass: ", tot_mass/CLHEP::kg);

  for (size_t k = 0; k < fMassFractionForVolumeArray.size(); k++) {
    // Normalize to one
    fMassFractionForVolumeArray.at(k) = fMassFractionForVolumeArray.at(k)/tot_mass;
    MGLog::OutFormat(MGLog::summary, "Fractional mass nr. %i is %g", k, fMassFractionForVolumeArray.at(k));
  }

  fVolumeArrayInitialized = true;
}

G4String MGGeneratorPrimary::ChooseVolumeFromList() {

  G4String sampled_vol;
  if (!fVolumeListInitialized) {
    this->InitializeVolumeListSampling();
    if (!fVolumeListInitialized) { // if it is still false, it means that the initialization failed
      MGLog::Out(MGLog::fatal, "Not able to initialize the volume list");
    }
  }

  G4double ran = G4UniformRand();
  // If ran is between W(k-1) and W(k), volume (k-1)-th is chosen, since W(0)=0
  G4double found_vol = false;
  size_t ifound = 0;
  for (size_t k = 1; k < fMassFractionForVolumeList.size() and !found_vol; k++) {
    if (ran > fMassFractionForVolumeList.at(k-1) and ran <= fMassFractionForVolumeList.at(k)) {
      found_vol = true; // it breaks when this happens
      ifound = k-1;
    }
  }

  MGLog::Out(MGLog::debug, "Found volume nr. ", ifound, ", random number: ", ran);
  G4int element_number = -1;

  if (ifound > fIDVolumeList.size()-1) {
    MGLog::Out(MGLog::error, "Index of found volume past list size, sampling from the last volume in the list.");
    element_number = fIDVolumeList.back();
  }
  else element_number = fIDVolumeList.at(ifound);

  return this->GetVolumeListName() + "_" + std::to_string(element_number);
}

G4String MGGeneratorPrimary::ChooseVolumeFromArray() {

  G4String sampled_vol;
  if (!fVolumeArrayInitialized) {
      InitializeVolumeArraySampling();
      if (!fVolumeArrayInitialized) { // if it is still false, it means that the initialization failed
        MGLog::Out(MGLog::fatal, "Not able to initialize the array");
      }
  }

  G4double ran = G4UniformRand();
  // if ran is between W(k-1) and W(k), volume (k-1)-th is chosen, since W(0)=0
  G4double found_vol = false;
  size_t ifound = 0;
  for (size_t k = 1; k < fMassFractionForVolumeArray.size() and !found_vol; k++) {
    if (ran > fMassFractionForVolumeArray.at(k-1) and ran <= fMassFractionForVolumeArray.at(k)) {
      found_vol = true; // it breaks when this happens
      ifound = k-1;
    }
  }
  MGLog::Out(MGLog::debug, "Found volume nr. ", ifound, ", random number: ", ran);

  if (ifound > fIDVolumeArray.size()-1) {
    MGLog::Out(MGLog::error, "Index of found volume past list size, sampling from the last volume in the list.");
    sampled_vol = fIDVolumeArray.back();
  }
  else sampled_vol = fIDVolumeArray.at(ifound);

  return sampled_vol;
}

void MGGeneratorPrimary::AddVolumeNumberToList(G4int det) {
  fIDVolumeList.push_back(det);
  MGLog::Out(MGLog::debug, "Added detector nr. ", det);
  MGLog::Out(MGLog::debug, "Size of the list: ", fIDVolumeList.size());
}

void MGGeneratorPrimary::AddVolumeNameToArray(G4String det) {
  fIDVolumeArray.push_back(det);
  MGLog::Out(MGLog::debug, "Added detector nr. ", det);
  MGLog::Out(MGLog::debug, "Size of the list: ", fIDVolumeArray.size());
}

// vim: tabstop=2 shiftwidth=2 expandtab

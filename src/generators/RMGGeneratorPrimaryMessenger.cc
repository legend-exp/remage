#include "RMGGeneratorPrimaryMessenger.hh"

#include "globals.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4PhysicalVolumeStore.hh"

#include "RMGVGenerator.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGGeneratorG4Gun.hh"
#include "RMGGeneratorSPS.hh"
#include "RMGLog.hh"

RMGGeneratorPrimaryMessenger::RMGGeneratorPrimaryMessenger(RMGGeneratorPrimary* generator) :
  fGeneratorPrimary(generator) {

  fGeneratorDirectory = new G4UIdirectory("/RMG/generator/");
  fGeneratorDirectory->SetGuidance("Control commands for generators:");
  fGeneratorDirectory->SetGuidance("/RMG/generator/select: Select generator.");

  fSelectCmd = new G4UIcmdWithAString("/RMG/generator/select", this);
  fSelectCmd->SetGuidance("Selects generator for events.");
  fSelectCmd->SetGuidance("Options are:");
  fSelectCmd->SetGuidance("G4gun: Standard G4 gun.");
  fSelectCmd->SetGuidance("SPS: Geant 4 SPS Generator.");
  fSelectCmd->SetGuidance("GSS: generic surface sampler.");
  fSelectCmd->SetCandidates("G4gun SPS");

  fNameCmd = new G4UIcmdWithoutParameter("/RMG/generator/name", this);
  fNameCmd->SetGuidance("Returns name of current generator.");

  fConfineCmd = new G4UIcmdWithAString("/RMG/generator/confine", this);
  fConfineCmd->SetGuidance("Selects confinement for the source.");
  fConfineCmd->SetGuidance("Options are:");
  fConfineCmd->SetGuidance("noconfined : source not confined");
  fConfineCmd->SetGuidance("volume : source confined in a (physical) volume.");
  fConfineCmd->SetGuidance("volumelist : source confined in a set of volumes with the same part name.");
  fConfineCmd->SetGuidance("volumearray : source confined in a set of volumes.");
  fConfineCmd->SetCandidates("noconfined volume volumelist volumearray surface surfacelist geometricalvolume geometricalsurface");

  fVolumeCmd = new G4UIcmdWithAString("/RMG/generator/volume", this);
  fVolumeCmd->SetGuidance("Selects the volume where the source is confined");
  fVolumeCmd->AvailableForStates(G4State_Init, G4State_Idle);

  fVolumeListCmd =  new G4UIcmdWithAString("/RMG/generator/volumelist", this);
  fVolumeListCmd -> SetGuidance("Selects the volumelist where the source is confined");

  fVolumeListFromCmd =  new G4UIcmdWithAnInteger("/RMG/generator/volumelistfrom", this);
  fVolumeListFromCmd -> SetGuidance("Selects the first volume in the list");

  fVolumeListToCmd =  new G4UIcmdWithAnInteger("/RMG/generator/volumelistto", this);
  fVolumeListToCmd -> SetGuidance("Selects the last volume in the list");

  fVolumeListAddCmd = new G4UIcmdWithAnInteger("/RMG/generator/volumelistadd", this);
  fVolumeListAddCmd->SetGuidance("Add a given volume number in the list");
  fVolumeListAddCmd->SetGuidance("Notice: this is ALTERNATIVE to give the list with from/to");

  fVolumeListClearCmd = new G4UIcmdWithoutParameter("/RMG/generator/volumelistclear",this);
  fVolumeListClearCmd->SetGuidance("Clear the current volume list");

  fVolumeArrayAddCmd = new G4UIcmdWithAString("/RMG/generator/volumearrayadd",this);
  fVolumeArrayAddCmd->SetGuidance("Add a given volume name in the array");
  fVolumeArrayAddCmd->SetGuidance("Notice: To add a given volume number to a volume list use /RMG/generator/volumelistadd");

  fPositionCmd = new G4UIcmdWith3VectorAndUnit("/RMG/generator/position", this);
  fPositionCmd->SetGuidance("Set starting (fixed) position of the particle.");
  fPositionCmd->SetParameterName("X", "Y", "Z", true, true);
  fPositionCmd->SetDefaultUnit("cm");
  fPositionCmd->SetUnitCategory("Length");
  fPositionCmd->SetUnitCandidates("microm mm cm m km");
}

RMGGeneratorPrimaryMessenger::~RMGGeneratorPrimaryMessenger() {
  delete fGeneratorDirectory;
  delete fNameCmd;
  delete fSelectCmd;
  delete fConfineCmd;
  delete fVolumeCmd;
  delete fVolumeListCmd;
  delete fVolumeListFromCmd;
  delete fVolumeListToCmd;
  delete fVolumeListAddCmd;
  delete fVolumeListClearCmd;
  delete fVolumeArrayAddCmd;
  delete fPositionCmd;
}

void RMGGeneratorPrimaryMessenger::SetNewValue(G4UIcommand* command, G4String new_values) {

  if (command == fSelectCmd) {
    if (new_values == "G4gun") {
      fGeneratorPrimary->SetRMGGenerator(new RMGGeneratorG4Gun);
    }
    else if (new_values == "SPS") {
      fGeneratorPrimary->SetRMGGenerator(new RMGGeneratorSPS);
    }
    else RMGLog::Out(RMGLog::fatal, "Unknown generator '", new_values, "'");
  }

  if (command == fConfineCmd) {
    if (new_values == "noconfined") {
      fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::noconfined);
      RMGLog::Out(RMGLog::detail, "Source not confined");
    }
    else if (new_values == "volume") {
      fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::volume);
      RMGLog::Out(RMGLog::detail, "Source confined in volume");
    }
    else if (new_values == "volumelist") {
      fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::volumelist);
      RMGLog::Out(RMGLog::detail, "Source confined in volume list");
    }
    else if (new_values == "volumearray") {
      fGeneratorPrimary->SetConfinementCode(RMGGeneratorPrimary::volumearray);
      RMGLog::Out(RMGLog::detail, "Source confined in volume array");
    }
  }

  if (command == fVolumeCmd) {

    fGeneratorPrimary->SetVolumeName(new_values);

    G4bool ifound = false;
    auto vol_store = G4PhysicalVolumeStore::GetInstance();
    auto n_volumes = vol_store->size();
    G4String candidate_list;
    for (size_t i = 0; i < n_volumes; i++) {
      candidate_list += vol_store->at(i)->GetName();
      candidate_list += ", ";
      if (vol_store->at(i)->GetName() == new_values) ifound = true;
    }

    if (ifound) {
      if (fGeneratorPrimary->GetConfinementCode() != RMGGeneratorPrimary::noconfined) {
        fGeneratorPrimary->SetVolumeName(new_values);
        RMGLog::Out(RMGLog::detail, "Source confined in ", new_values);
      }
      else RMGLog::Out(RMGLog::warning, "Source not confined: nothing happens ");
    }
    else {
      RMGLog::Out(RMGLog::warning, "Volume not found ");
      RMGLog::Out(RMGLog::warning, "The list of volumes is: ", candidate_list);
    }
  }

  if (command == fVolumeListFromCmd) {
    fGeneratorPrimary -> SetVolumeListFrom(fVolumeListFromCmd->GetNewIntValue(new_values));
    fGeneratorPrimary->SetVolumeListInitialized(false);
  }

  if (command == fVolumeListToCmd) {
    fGeneratorPrimary -> SetVolumeListTo(fVolumeListToCmd->GetNewIntValue(new_values));
    fGeneratorPrimary->SetVolumeListInitialized(false);
  }
  if (command == fVolumeListAddCmd){
    fGeneratorPrimary->AddVolumeNumberToList(fVolumeListAddCmd->GetNewIntValue(new_values));
    fGeneratorPrimary->SetVolumeListInitialized(false);
  }
  if (command == fVolumeListClearCmd){
    fGeneratorPrimary->ClearList();
    fGeneratorPrimary->SetVolumeListInitialized(false);
  }
  if (command == fVolumeListCmd) {
    G4bool ifound = false;
    G4bool jfound = false;
    auto vol_store = G4PhysicalVolumeStore::GetInstance();
    auto n_volumes = vol_store->size();
    G4String start_volume = new_values + "_" + std::to_string(fGeneratorPrimary->GetVolumeListFrom());
    G4String end_volume = new_values + "_" + std::to_string(fGeneratorPrimary->GetVolumeListTo());

    G4String candidate_list;
    for (size_t i = 0; i < n_volumes; i++) {
      candidate_list += vol_store->at(i)->GetName();
      candidate_list += ", ";
      if (vol_store->at(i)->GetName() == start_volume) ifound = true;
      if (vol_store->at(i)->GetName() == end_volume) jfound = true;
    }

    if (ifound and jfound) {
      if (fGeneratorPrimary->GetConfinementCode() != RMGGeneratorPrimary::noconfined) {
        fGeneratorPrimary->SetVolumeListName(new_values);
        RMGLog::Out(RMGLog::detail, "Source confined in ", start_volume, " to ", end_volume);
      }
      else RMGLog::Out(RMGLog::warning, "Source not confined: nothing happens");
    }
    else {
      if (ifound == false) RMGLog::Out(RMGLog::warning, "Volume ", start_volume, " not found ");
      if (jfound == false) RMGLog::Out(RMGLog::warning, "Volume ", end_volume, " not found ");
      RMGLog::Out(RMGLog::warning, "The list of volumes is: ", candidate_list);
    }
    fGeneratorPrimary->SetVolumeListInitialized(false);
  }
  if (command == fVolumeArrayAddCmd) {
    fGeneratorPrimary->AddVolumeNameToArray(new_values);
  }
  if (command == fPositionCmd) {
    fGeneratorPrimary->GetRMGGenerator()->SetParticlePosition(fPositionCmd->GetNew3VectorValue(new_values));
    RMGLog::Out(RMGLog::detail, "Default starting position set to ",
        fPositionCmd->GetNew3VectorValue(new_values)/CLHEP::cm, "cm");
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "MGGeneratorPrimaryMessenger.hh"

#include "globals.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4PhysicalVolumeStore.hh"

#include "MGVGenerator.hh"
#include "MGGeneratorPrimary.hh"
#include "MGGeneratorG4Gun.hh"
#include "MGGeneratorSPS.hh"
#include "MGLog.hh"

MGGeneratorPrimaryMessenger::MGGeneratorPrimaryMessenger(MGGeneratorPrimary* generator) :
  fGeneratorPrimary(generator) {

  fGeneratorDirectory = new G4UIdirectory("/MG/generator/");
  fGeneratorDirectory->SetGuidance("Control commands for generators:");
  fGeneratorDirectory->SetGuidance("/MG/generator/select: Select generator.");

  fSelectCmd = new G4UIcmdWithAString("/MG/generator/select", this);
  fSelectCmd->SetGuidance("Selects generator for events.");
  fSelectCmd->SetGuidance("Options are:");
  fSelectCmd->SetGuidance("G4gun: Standard G4 gun.");
  fSelectCmd->SetGuidance("SPS: Geant 4 SPS Generator.");
  fSelectCmd->SetGuidance("GSS: generic surface sampler.");
  fSelectCmd->SetCandidates("G4gun SPS");

  fNameCmd = new G4UIcmdWithoutParameter("/MG/generator/name", this);
  fNameCmd->SetGuidance("Returns name of current generator.");

  fConfineCmd = new G4UIcmdWithAString("/MG/generator/confine", this);
  fConfineCmd->SetGuidance("Selects confinement for the source.");
  fConfineCmd->SetGuidance("Options are:");
  fConfineCmd->SetGuidance("noconfined : source not confined");
  fConfineCmd->SetGuidance("volume : source confined in a (physical) volume.");
  fConfineCmd->SetGuidance("volumelist : source confined in a set of volumes with the same part name.");
  fConfineCmd->SetGuidance("volumearray : source confined in a set of volumes.");
  fConfineCmd->SetCandidates("noconfined volume volumelist volumearray surface surfacelist geometricalvolume geometricalsurface");

  fVolumeCmd = new G4UIcmdWithAString("/MG/generator/volume", this);
  fVolumeCmd->SetGuidance("Selects the volume where the source is confined");
  fVolumeCmd->AvailableForStates(G4State_Init, G4State_Idle);

  fVolumeListCmd =  new G4UIcmdWithAString("/MG/generator/volumelist", this);
  fVolumeListCmd -> SetGuidance("Selects the volumelist where the source is confined");

  fVolumeListFromCmd =  new G4UIcmdWithAnInteger("/MG/generator/volumelistfrom", this);
  fVolumeListFromCmd -> SetGuidance("Selects the first volume in the list");

  fVolumeListToCmd =  new G4UIcmdWithAnInteger("/MG/generator/volumelistto", this);
  fVolumeListToCmd -> SetGuidance("Selects the last volume in the list");

  fVolumeListAddCmd = new G4UIcmdWithAnInteger("/MG/generator/volumelistadd", this);
  fVolumeListAddCmd->SetGuidance("Add a given volume number in the list");
  fVolumeListAddCmd->SetGuidance("Notice: this is ALTERNATIVE to give the list with from/to");

  fVolumeListClearCmd = new G4UIcmdWithoutParameter("/MG/generator/volumelistclear",this);
  fVolumeListClearCmd->SetGuidance("Clear the current volume list");

  fVolumeArrayAddCmd = new G4UIcmdWithAString("/MG/generator/volumearrayadd",this);
  fVolumeArrayAddCmd->SetGuidance("Add a given volume name in the array");
  fVolumeArrayAddCmd->SetGuidance("Notice: To add a given volume number to a volume list use /MG/generator/volumelistadd");

  fPositionCmd = new G4UIcmdWith3VectorAndUnit("/MG/generator/position", this);
  fPositionCmd->SetGuidance("Set starting (fixed) position of the particle.");
  fPositionCmd->SetParameterName("X", "Y", "Z", true, true);
  fPositionCmd->SetDefaultUnit("cm");
  fPositionCmd->SetUnitCategory("Length");
  fPositionCmd->SetUnitCandidates("microm mm cm m km");
}

MGGeneratorPrimaryMessenger::~MGGeneratorPrimaryMessenger() {
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

void MGGeneratorPrimaryMessenger::SetNewValue(G4UIcommand* command, G4String new_values) {

  if (command == fSelectCmd) {
    if (new_values == "G4gun") {
      fGeneratorPrimary->SetMGGenerator(new MGGeneratorG4Gun);
    }
    else if (new_values == "SPS") {
      fGeneratorPrimary->SetMGGenerator(new MGGeneratorSPS);
    }
    else MGLog::Out(MGLog::fatal, "Unknown generator '", new_values, "'");
  }

  if (command == fConfineCmd) {
    if (new_values == "noconfined") {
      fGeneratorPrimary->SetConfinementCode(MGGeneratorPrimary::noconfined);
      MGLog::Out(MGLog::detail, "Source not confined");
    }
    else if(new_values == "volume") {
      fGeneratorPrimary->SetConfinementCode(MGGeneratorPrimary::volume);
      MGLog::Out(MGLog::detail, "Source confined in volume");
    }
    else if(new_values == "volumelist") {
      fGeneratorPrimary->SetConfinementCode(MGGeneratorPrimary::volumelist);
      MGLog::Out(MGLog::detail, "Source confined in volume list");
    }
    else if(new_values == "volumearray") {
      fGeneratorPrimary->SetConfinementCode(MGGeneratorPrimary::volumearray);
      MGLog::Out(MGLog::detail, "Source confined in volume array");
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
      if (fGeneratorPrimary->GetConfinementCode() != MGGeneratorPrimary::noconfined) {
        fGeneratorPrimary->SetVolumeName(new_values);
        MGLog::Out(MGLog::detail, "Source confined in ", new_values);
      }
      else MGLog::Out(MGLog::warning, "Source not confined: nothing happens ");
    }
    else {
      MGLog::Out(MGLog::warning, "Volume not found ");
      MGLog::Out(MGLog::warning, "The list of volumes is: ", candidate_list);
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
      if (fGeneratorPrimary->GetConfinementCode() != MGGeneratorPrimary::noconfined) {
        fGeneratorPrimary->SetVolumeListName(new_values);
        MGLog::Out(MGLog::detail, "Source confined in ", start_volume, " to ", end_volume);
      }
      else MGLog::Out(MGLog::warning, "Source not confined: nothing happens");
    }
    else {
      if (ifound == false) MGLog::Out(MGLog::warning, "Volume ", start_volume, " not found ");
      if (jfound == false) MGLog::Out(MGLog::warning, "Volume ", end_volume, " not found ");
      MGLog::Out(MGLog::warning, "The list of volumes is: ", candidate_list);
    }
    fGeneratorPrimary->SetVolumeListInitialized(false);
  }
  if (command == fVolumeArrayAddCmd) {
    fGeneratorPrimary->AddVolumeNameToArray(new_values);
  }
  if (command == fPositionCmd) {
    fGeneratorPrimary->GetMGGenerator()->SetParticlePosition(fPositionCmd->GetNew3VectorValue(new_values));
    MGLog::Out(MGLog::detail, "Default starting position set to ",
        fPositionCmd->GetNew3VectorValue(new_values)/CLHEP::cm, "cm");
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

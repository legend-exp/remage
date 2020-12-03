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
      fGeneratorPrimary->SetGenerator(new RMGGeneratorG4Gun);
    }
    else if (new_values == "SPS") {
      fGeneratorPrimary->SetGenerator(new RMGGeneratorSPS);
    }
    else RMGLog::Out(RMGLog::fatal, "Unknown generator '", new_values, "'");
  }

  // else if...

}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "RMGGeneratorVolumeConfinementMessenger.hh"

#include "globals.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIdirectory.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

#include "RMGGeneratorVolumeConfinement.hh"
#include "RMGLog.hh"

RMGGeneratorVolumeConfinementMessenger::RMGGeneratorVolumeConfinementMessenger(RMGGeneratorVolumeConfinement* generator):
  fSampler(generator) {

  fSamplerDirectory = new G4UIdirectory("/RMG/generator/Sampling/");
  fSamplerDirectory->SetGuidance("Control commands for the geometry sampler");

  fVolumeDirectory =  new G4UIdirectory("/RMG/generator/Sampling/volume/");
  fVolumeDirectory->SetGuidance("Control commands for the volume geometry sampler");

  fVolumeNameCmd = new G4UIcmdWithAString("/RMG/generator/Sampling/volume/name", this);
  fVolumeNameCmd->SetGuidance("Selects the name of the geometrical volume for sampling");
  fVolumeNameCmd->SetGuidance("Options are: ");
  fVolumeNameCmd->SetGuidance("Sphere: sample from the volume of a spherical shell");
  fVolumeNameCmd->SetGuidance("Cylinder: sample from the volume of a cylindrical shell");
  fVolumeNameCmd->SetGuidance("Box: sample from the volume of a box");
  fVolumeNameCmd->SetCandidates("Sphere Cylinder Box");

  fInnerSphereRadiusVolCmd = new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/innerSphereRadius",this);
  fInnerSphereRadiusVolCmd->SetGuidance("Set the inner radius for sphere (default: 0 cm)");
  fInnerSphereRadiusVolCmd->SetDefaultUnit("cm");
  fInnerSphereRadiusVolCmd->SetUnitCategory("Length");
  fInnerSphereRadiusVolCmd->SetUnitCandidates("microm mm cm m km");

  fOuterSphereRadiusVolCmd = new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/outerSphereRadius",this);
  fOuterSphereRadiusVolCmd->SetGuidance("Set the outer radius for sphere ");
  fOuterSphereRadiusVolCmd->SetDefaultUnit("cm");
  fOuterSphereRadiusVolCmd->SetUnitCategory("Length");
  fOuterSphereRadiusVolCmd->SetUnitCandidates("microm mm cm m km");

  fInnerCylinderRadiusVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/innerCylinderRadius",this);
  fInnerCylinderRadiusVolCmd->SetGuidance("Set the inner radius for cylinder volume sampling (default: 0 cm)");
  fInnerCylinderRadiusVolCmd->SetDefaultUnit("cm");
  fInnerCylinderRadiusVolCmd->SetUnitCategory("Length");
  fInnerCylinderRadiusVolCmd->SetUnitCandidates("microm mm cm m km");

  fOuterCylinderRadiusVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/outerCylinderRadius",this);
  fOuterCylinderRadiusVolCmd->SetGuidance("Set the outer radius for cylinder volume sampling");
  fOuterCylinderRadiusVolCmd->SetDefaultUnit("cm");
  fOuterCylinderRadiusVolCmd->SetUnitCategory("Length");
  fOuterCylinderRadiusVolCmd->SetUnitCandidates("microm mm cm m km");

  fCylinderHeightVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/cylinderHeight",this);
  fCylinderHeightVolCmd->SetGuidance("Set the height for cylinder volume sampling");
  fCylinderHeightVolCmd->SetDefaultUnit("cm");
  fCylinderHeightVolCmd->SetUnitCategory("Length");
  fCylinderHeightVolCmd->SetUnitCandidates("microm mm cm m km");

  fCylinderStartingAngleVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/cylinderStartingAngle",this);
  fCylinderStartingAngleVolCmd->SetGuidance("Set the starting angle for cylinder volume sampling (default: 0 deg)");
  fCylinderStartingAngleVolCmd->SetDefaultUnit("deg");
  fCylinderStartingAngleVolCmd->SetUnitCategory("Angle");
  fCylinderStartingAngleVolCmd->SetUnitCandidates("deg");

  fCylinderSpanningAngleVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/cylinderSpanningAngle",this);
  fCylinderSpanningAngleVolCmd->SetGuidance("Set the spanning angle for cylinder volume sampling (default: 360 deg )");
  fCylinderSpanningAngleVolCmd->SetDefaultUnit("deg");
  fCylinderSpanningAngleVolCmd->SetUnitCategory("Angle");
  fCylinderSpanningAngleVolCmd->SetUnitCandidates("deg");

  fBoxXLengthVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/boxXLength",this);
  fBoxXLengthVolCmd->SetGuidance("Set the X-dimension for box volume sampling");
  fBoxXLengthVolCmd->SetDefaultUnit("cm");
  fBoxXLengthVolCmd->SetUnitCategory("Length");
  fBoxXLengthVolCmd->SetUnitCandidates("microm mm cm m km");

  fBoxYLengthVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/boxYLength",this);
  fBoxYLengthVolCmd->SetGuidance("Set the Y-dimension for box volume sampling");
  fBoxYLengthVolCmd->SetDefaultUnit("cm");
  fBoxYLengthVolCmd->SetUnitCategory("Length");
  fBoxYLengthVolCmd->SetUnitCandidates("microm mm cm m km");

  fBoxZLengthVolCmd =  new G4UIcmdWithADoubleAndUnit("/RMG/generator/Sampling/volume/boxZLength",this);
  fBoxZLengthVolCmd->SetGuidance("Set the X-dimension for box volume sampling");
  fBoxZLengthVolCmd->SetDefaultUnit("cm");
  fBoxZLengthVolCmd->SetUnitCategory("Length");
  fBoxZLengthVolCmd->SetUnitCandidates("microm mm cm m km");

  fVolCenterCmd =  new G4UIcmdWith3VectorAndUnit("/RMG/generator/Sampling/volume/offsetCenterPos",this);
  fVolCenterCmd->SetGuidance("Set the position of the volume center");
  fVolCenterCmd->SetDefaultUnit("cm");
  fVolCenterCmd->SetUnitCategory("Length");
  fVolCenterCmd->SetUnitCandidates("microm mm cm m km");

  fNPositionsamplingMaxCmd =  new G4UIcmdWithAnInteger("/RMG/generator/Sampling/NPositionsamplingMax",this);
  fNPositionsamplingMaxCmd->SetGuidance("Set the maximum number of tries used to find a vertex inside a volume/ on a surface.");
}

RMGGeneratorVolumeConfinementMessenger::~RMGGeneratorVolumeConfinementMessenger() {
  delete fSamplerDirectory;
  delete fVolumeDirectory;
  delete fVolumeNameCmd;
  delete fInnerSphereRadiusVolCmd;
  delete fOuterSphereRadiusVolCmd;
  delete fInnerDiskRadiusCmd;
  delete fOuterDiskRadiusCmd;
  delete fCylinderHeightSurfCmd;
  delete fInnerCylinderRadiusVolCmd;
  delete fOuterCylinderRadiusVolCmd;
  delete fCylinderHeightVolCmd;
  delete fCylinderStartingAngleVolCmd;
  delete fCylinderSpanningAngleVolCmd;
  delete fBoxXLengthVolCmd;
  delete fBoxYLengthVolCmd;
  delete fBoxZLengthVolCmd;
  delete fVolCenterCmd;
  delete fNPositionsamplingMaxCmd;
}

void RMGGeneratorVolumeConfinementMessenger::SetNewValue(G4UIcommand *command, G4String new_values) {
  // if (command == fVolumeNameCmd) fSampler->SetGeometricalVolumeName(new_values);
  // else if (command == fInnerSphereRadiusVolCmd) fSampler->SetInnerSphereRadius(fInnerSphereRadiusVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fOuterSphereRadiusVolCmd) fSampler->SetOuterSphereRadius(fOuterSphereRadiusVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fInnerDiskRadiusCmd) fSampler->SetInnerDiskRadius(fInnerDiskRadiusCmd->GetNewDoubleValue(new_values));
  // else if (command == fOuterDiskRadiusCmd) fSampler->SetOuterDiskRadius(fOuterDiskRadiusCmd->GetNewDoubleValue(new_values));
  // else if (command == fCylinderHeightSurfCmd) fSampler->SetCylinderHeight(fCylinderHeightSurfCmd->GetNewDoubleValue(new_values));
  // else if (command == fInnerCylinderRadiusVolCmd) fSampler->SetInnerCylinderRadius(fInnerCylinderRadiusVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fOuterCylinderRadiusVolCmd) fSampler->SetOuterCylinderRadius(fOuterCylinderRadiusVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fCylinderHeightVolCmd) fSampler->SetCylinderHeight(fCylinderHeightVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fCylinderStartingAngleVolCmd) fSampler->SetCylinderStartingAngle(fCylinderStartingAngleVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fCylinderSpanningAngleVolCmd) fSampler->SetCylinderSpanningAngle(fCylinderSpanningAngleVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fBoxXLengthVolCmd) fSampler->SetXLengthBox(fBoxXLengthVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fBoxYLengthVolCmd) fSampler->SetYLengthBox(fBoxYLengthVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fBoxZLengthVolCmd) fSampler->SetZLengthBox(fBoxZLengthVolCmd->GetNewDoubleValue(new_values));
  // else if (command == fVolCenterCmd) fSampler->SetGeometricalVolumeCenter(fVolCenterCmd->GetNew3VectorValue(new_values));
  // else if (command == fNPositionsamplingMaxCmd) fSampler->SetNPositionsamplingMax( fNPositionsamplingMaxCmd->GetNewIntValue(new_values));
  // else RMGLog::Out(RMGLog::error, "Command ", command->GetTitle(), "not known");
}

// vim: tabstop=2 shiftwidth=2 expandtab

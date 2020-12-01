#ifndef _RMGGENERATORPOSITIONSAMPLINGMESSENGER_HH_
#define _RMGGENERATORPOSITIONSAMPLINGMESSENGER_HH_

#include "globals.hh"
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIcmdWithAnInteger; 
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithADoubleAndUnit;
class RMGGeneratorPositionSampling;
class RMGGeneratorPositionSamplingMessenger : public G4UImessenger {

 public:

   RMGGeneratorPositionSamplingMessenger(RMGGeneratorPositionSampling* generator);
   ~RMGGeneratorPositionSamplingMessenger();

   RMGGeneratorPositionSamplingMessenger           (RMGGeneratorPositionSamplingMessenger const&) = delete;
   RMGGeneratorPositionSamplingMessenger& operator=(RMGGeneratorPositionSamplingMessenger const&) = delete;
   RMGGeneratorPositionSamplingMessenger           (RMGGeneratorPositionSamplingMessenger&&)      = delete;
   RMGGeneratorPositionSamplingMessenger& operator=(RMGGeneratorPositionSamplingMessenger&&)      = delete;

   void SetNewValue(G4UIcommand *command, G4String new_values) override;

 private:

   RMGGeneratorPositionSampling* fSampler;

   G4UIdirectory*             fSamplerDirectory;
   G4UIdirectory*             fSurfaceDirectory;
   G4UIdirectory*             fVolumeDirectory;
   G4UIcmdWithAString*        fVolumeNameCmd;
   G4UIcmdWithAString*        fSurfaceNameCmd;
   G4UIcmdWithADoubleAndUnit* fInnerSphereRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fOuterSphereRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fSphereRadiusSurfCmd;
   G4UIcmdWithADoubleAndUnit* fInnerDiskRadiusCmd;
   G4UIcmdWithADoubleAndUnit* fOuterDiskRadiusCmd;
   G4UIcmdWithADoubleAndUnit* fInnerCylinderRadiusSurfCmd;
   G4UIcmdWithADoubleAndUnit* fOuterCylinderRadiusSurfCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderHeightSurfCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderStartingAngleSurfCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderSpanningAngleSurfCmd;
   G4UIcmdWithADoubleAndUnit* fInnerCylinderRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fOuterCylinderRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderHeightVolCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderStartingAngleVolCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderSpanningAngleVolCmd;
   G4UIcmdWithADoubleAndUnit* fBoxXLengthVolCmd;
   G4UIcmdWithADoubleAndUnit* fBoxYLengthVolCmd;
   G4UIcmdWithADoubleAndUnit* fBoxZLengthVolCmd;
   G4UIcmdWithADoubleAndUnit* fVolCenterXCmd;
   G4UIcmdWithADoubleAndUnit* fVolCenterYCmd;
   G4UIcmdWithADoubleAndUnit* fVolCenterZCmd;
   G4UIcmdWithADoubleAndUnit* fSurfCenterXCmd;
   G4UIcmdWithADoubleAndUnit* fSurfCenterYCmd;
   G4UIcmdWithADoubleAndUnit* fSurfCenterZCmd;
   G4UIcmdWithAnInteger*      fNPositionsamplingMaxCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

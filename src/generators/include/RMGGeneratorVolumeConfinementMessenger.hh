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
class RMGGeneratorVolumeConfinement;
class RMGGeneratorVolumeConfinementMessenger : public G4UImessenger {

 public:

   RMGGeneratorVolumeConfinementMessenger(RMGGeneratorVolumeConfinement* generator);
   ~RMGGeneratorVolumeConfinementMessenger();

   RMGGeneratorVolumeConfinementMessenger           (RMGGeneratorVolumeConfinementMessenger const&) = delete;
   RMGGeneratorVolumeConfinementMessenger& operator=(RMGGeneratorVolumeConfinementMessenger const&) = delete;
   RMGGeneratorVolumeConfinementMessenger           (RMGGeneratorVolumeConfinementMessenger&&)      = delete;
   RMGGeneratorVolumeConfinementMessenger& operator=(RMGGeneratorVolumeConfinementMessenger&&)      = delete;

   void SetNewValue(G4UIcommand *command, G4String new_values) override;

 private:

   RMGGeneratorVolumeConfinement* fSampler;

   G4UIdirectory*             fSamplerDirectory;
   G4UIdirectory*             fVolumeDirectory;
   G4UIcmdWithAString*        fVolumeNameCmd;
   G4UIcmdWithADoubleAndUnit* fInnerSphereRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fOuterSphereRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fInnerDiskRadiusCmd;
   G4UIcmdWithADoubleAndUnit* fOuterDiskRadiusCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderHeightSurfCmd;
   G4UIcmdWithADoubleAndUnit* fInnerCylinderRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fOuterCylinderRadiusVolCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderHeightVolCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderStartingAngleVolCmd;
   G4UIcmdWithADoubleAndUnit* fCylinderSpanningAngleVolCmd;
   G4UIcmdWithADoubleAndUnit* fBoxXLengthVolCmd;
   G4UIcmdWithADoubleAndUnit* fBoxYLengthVolCmd;
   G4UIcmdWithADoubleAndUnit* fBoxZLengthVolCmd;
   G4UIcmdWith3VectorAndUnit* fVolCenterCmd;
   G4UIcmdWithAnInteger*      fNPositionsamplingMaxCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

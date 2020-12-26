#ifndef _RMG_GENERATOR_POSITION_SAMPLING_MESSENGER_HH_
#define _RMG_GENERATOR_POSITION_SAMPLING_MESSENGER_HH_

#include <vector>
#include <memory>

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

class G4UIcommand;
class RMGGeneratorVolumeConfinement;
class RMGGeneratorVolumeConfinementMessenger : public G4UImessenger {

 public:

   RMGGeneratorVolumeConfinementMessenger(RMGGeneratorVolumeConfinement* generator);
   ~RMGGeneratorVolumeConfinementMessenger() = default;

   RMGGeneratorVolumeConfinementMessenger           (RMGGeneratorVolumeConfinementMessenger const&) = delete;
   RMGGeneratorVolumeConfinementMessenger& operator=(RMGGeneratorVolumeConfinementMessenger const&) = delete;
   RMGGeneratorVolumeConfinementMessenger           (RMGGeneratorVolumeConfinementMessenger&&)      = delete;
   RMGGeneratorVolumeConfinementMessenger& operator=(RMGGeneratorVolumeConfinementMessenger&&)      = delete;

   void SetNewValue(G4UIcommand *command, G4String new_values) override;

 private:

   RMGGeneratorVolumeConfinement* fSampler;

   std::vector<std::unique_ptr<G4UIdirectory>> fDirectories;

   std::unique_ptr<G4UIcmdWithAString>        fSamplingModeCmd;
   std::unique_ptr<G4UIcmdWithAString>        fBoundingSolidTypeCmd;

   std::unique_ptr<G4UIcmdWithAString>        fAddPhysVolCmd;
   std::unique_ptr<G4UIcmdWithAString>        fAddGeomVolCmd;

   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fSphereInnerRadiusVolCmd;
   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fSphereOuterRadiusVolCmd;

   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fCylinderInnerRadiusVolCmd;
   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fCylinderOuterRadiusVolCmd;
   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fCylinderHeightVolCmd;

   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fCylinderStartingAngleVolCmd;
   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fCylinderSpanningAngleVolCmd;
   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fBoxXLengthVolCmd;
   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fBoxYLengthVolCmd;
   std::unique_ptr<G4UIcmdWithADoubleAndUnit> fBoxZLengthVolCmd;

   std::unique_ptr<G4UIcmdWith3VectorAndUnit> fGeomVolCenterCmd;
   std::unique_ptr<G4UIcmdWithAnInteger>      fNPositionsamplingMaxCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

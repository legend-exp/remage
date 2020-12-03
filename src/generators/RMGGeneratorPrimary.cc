#include "RMGGeneratorPrimary.hh"

#include "RMGGeneratorPrimaryMessenger.hh"
#include "RMGVGeneratorPrimaryPosition.hh"
#include "RMGGeneratorVolumeConfinement.hh"
// #include "RMGGeneratorGeometricalVolumeConfinement.hh"
#include "RMGVGenerator.hh"
#include "RMGLog.hh"

RMGGeneratorPrimary::RMGGeneratorPrimary():
  fConfinementCode(ConfinementCode::kUnConfined) {

  fG4Messenger = new RMGGeneratorPrimaryMessenger(this);
}

RMGGeneratorPrimary::~RMGGeneratorPrimary() {
  delete fPrimaryPositionGenerator;
  delete fRMGGenerator;
  delete fG4Messenger;
}

void RMGGeneratorPrimary::GeneratePrimaries(G4Event* event) {

  if (!fPrimaryPositionGenerator) RMGLog::Out(RMGLog::fatal, "No primary position generator specified!");
  if (!fRMGGenerator) RMGLog::Out(RMGLog::fatal, "No generator specified!");

  fRMGGenerator->SetParticlePosition(fPrimaryPositionGenerator->ShootPrimaryPosition());
  fRMGGenerator->GeneratePrimaryVertex(event);
}

void RMGGeneratorPrimary::SetConfinementCode(RMGGeneratorPrimary::ConfinementCode code) {

  fConfinementCode = code;

  switch (fConfinementCode) {
    case ConfinementCode::kUnConfined :
      fPrimaryPositionGenerator = new RMGVGeneratorPrimaryPosition("DummyGenerator");
      break;
    case ConfinementCode::kVolume :
    case ConfinementCode::kVolumeList :
    case ConfinementCode::kSurface :
    case ConfinementCode::kSurfaceList :
      fPrimaryPositionGenerator = new RMGGeneratorVolumeConfinement();
      break;
    case ConfinementCode::kGeometricalVolume :
      // fPrimaryPositionGenerator = new RMGGeneratorGeometricalVolumeConfinement();
      break;
    default : RMGLog::Out(RMGLog::fatal, "No sampling strategy for confinement '",
                                         fConfinementCode, "' specified (implement me)");
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

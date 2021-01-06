#include "RMGGeneratorPrimary.hh"

#include "RMGGeneratorPrimaryMessenger.hh"
#include "RMGVGeneratorPrimaryPosition.hh"
#include "RMGGeneratorVolumeConfinement.hh"
#include "RMGVGenerator.hh"
#include "RMGLog.hh"

RMGGeneratorPrimary::RMGGeneratorPrimary():
  fConfinementCode(ConfinementCode::kUnConfined) {

  fG4Messenger = std::unique_ptr<RMGGeneratorPrimaryMessenger>(new RMGGeneratorPrimaryMessenger(this));
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
      fPrimaryPositionGenerator = std::unique_ptr<RMGVGeneratorPrimaryPosition>(new RMGVGeneratorPrimaryPosition("DummyGenerator"));
      break;
    case ConfinementCode::kVolume :
      fPrimaryPositionGenerator = std::unique_ptr<RMGGeneratorVolumeConfinement>(new RMGGeneratorVolumeConfinement());
      break;
    default : RMGLog::Out(RMGLog::fatal, "No sampling strategy for confinement '",
                                         fConfinementCode, "' specified (implement me)");
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

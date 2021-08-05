#include "RMGGeneratorPrimary.hh"

#include "RMGVGeneratorPrimaryPosition.hh"
#include "RMGGeneratorVolumeConfinement.hh"
#include "RMGVGenerator.hh"
#include "RMGGeneratorG4Gun.hh"
#include "RMGGeneratorSPS.hh"
#if RMG_HAS_BXDECAY0
#include "RMGGeneratorDecay0.hh"
#endif
#include "RMGLog.hh"
#include "G4GenericMessenger.hh"

#include "magic_enum/magic_enum.hpp"

RMGGeneratorPrimary::RMGGeneratorPrimary():
  fConfinementCode(RMGGeneratorPrimary::ConfinementCode::kUnConfined),
  fGenerator(RMGGeneratorPrimary::Generator::kUndefined) {

  this->DefineCommands();
}

void RMGGeneratorPrimary::GeneratePrimaries(G4Event* event) {

  if (!fPrimaryPositionGenerator) RMGLog::Out(RMGLog::fatal, "No primary position generator specified!");
  if (!fGeneratorObj) RMGLog::Out(RMGLog::fatal, "No generator specified!");

  fGeneratorObj->SetParticlePosition(fPrimaryPositionGenerator->ShootPrimaryPosition());
  fGeneratorObj->GeneratePrimaryVertex(event);
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

void RMGGeneratorPrimary::SetConfinementCodeString(G4String code) {
  auto result = magic_enum::enum_cast<RMGGeneratorPrimary::ConfinementCode>(code);
  if (result.has_value()) RMGGeneratorPrimary::SetConfinementCode(result.value());
  else RMGLog::Out(RMGLog::error, "Illegal confinement code '", code, "'");
}

void RMGGeneratorPrimary::SetGenerator(RMGGeneratorPrimary::Generator gen) {

  fGenerator = gen;

  switch (fGenerator) {
    case RMGGeneratorPrimary::Generator::kG4gun:
      fGeneratorObj = std::make_unique<RMGGeneratorG4Gun>();
      break;
    case RMGGeneratorPrimary::Generator::kSPS:
      fGeneratorObj = std::make_unique<RMGGeneratorSPS>();
      break;
    case RMGGeneratorPrimary::Generator::kBxDecay0:
#if RMG_HAS_BXDECAY0
      fGeneratorObj = std::make_unique<RMGGeneratorG4Gun>();
#else
      RMGLog::Out(RMGLog::fatal, "BxDecay0 not available, please recompile remage with -DRMG_USE_BXDECAY0=ON");
#endif
      break;
    case RMGGeneratorPrimary::Generator::kUndefined:
    case RMGGeneratorPrimary::Generator::kUserDefined:
      break;
    default:
      RMGLog::Out(RMGLog::fatal, "No known implementation for generator '",
          fGenerator, "' (implement me)");
  }
}

void RMGGeneratorPrimary::SetGeneratorString(G4String gen) {
  auto result = magic_enum::enum_cast<RMGGeneratorPrimary::Generator>(gen);
  if (result.has_value()) RMGGeneratorPrimary::SetGenerator(result.value());
  else RMGLog::Out(RMGLog::error, "Illegal generator name '", gen, "'");
}

void RMGGeneratorPrimary::SetUserGenerator(RMGVGenerator* gen) {

  fGenerator = RMGGeneratorPrimary::Generator::kUserDefined;
  fGeneratorObj = std::unique_ptr<RMGVGenerator>(gen);
}

void RMGGeneratorPrimary::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/",
      "Commands for controlling generators");

  fMessenger->DeclareMethod("Confine", &RMGGeneratorPrimary::SetConfinementCodeString)
    .SetGuidance("Select primary confinement strategy")
    .SetParameterName("strategy", false)
    .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("Select", &RMGGeneratorPrimary::SetGeneratorString)
    .SetGuidance("Select event generator")
    .SetParameterName("generator", false)
    .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

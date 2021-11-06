#include "RMGGeneratorPrimary.hh"

#include "ProjectInfo.hh"
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

#include "RMGTools.hh"

RMGGeneratorPrimary::RMGGeneratorPrimary():
  fConfinementCode(RMGGeneratorPrimary::ConfinementCode::kUnConfined),
  fGenerator(RMGGeneratorPrimary::Generator::kUndefined) {

  this->DefineCommands();
}

void RMGGeneratorPrimary::GeneratePrimaries(G4Event* event) {

  if (!fPrimaryPositionGenerator) RMGLog::Out(RMGLog::fatal, "No primary position generator (confinement) specified!");
  if (!fGeneratorObj) RMGLog::Out(RMGLog::fatal, "No primary generator specified!");

  // HACK: The BxDecay0 generator takes the responsibility for shooting the primary vertex position,
  // and this conflicts with the design I had in mind here (i.e. that a RMGVGenerator is instructed
  // about the vertex position from outside, in particular in this function here).
  if (fGenerator != RMGGeneratorPrimary::Generator::kBxDecay0) {
    auto vertex = fPrimaryPositionGenerator->ShootPrimaryPosition();
    RMGLog::OutDev(RMGLog::debug, "Primary vertex position: ", vertex/CLHEP::cm, " cm");
    fGeneratorObj->SetParticlePosition(vertex);
  }
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
  RMGLog::OutFormat(RMGLog::debug, "Primary vertex confinement strategy set to {}", magic_enum::enum_name<RMGGeneratorPrimary::ConfinementCode>(code));
}

void RMGGeneratorPrimary::SetGenerator(RMGGeneratorPrimary::Generator gen) {

  fGenerator = gen;

  switch (fGenerator) {
    case RMGGeneratorPrimary::Generator::kG4gun :
      fGeneratorObj = std::make_unique<RMGGeneratorG4Gun>();
      break;
    case RMGGeneratorPrimary::Generator::kSPS :
      fGeneratorObj = std::make_unique<RMGGeneratorSPS>();
      break;
    case RMGGeneratorPrimary::Generator::kBxDecay0 :
#if RMG_HAS_BXDECAY0
      fGeneratorObj = std::make_unique<RMGGeneratorDecay0>(fPrimaryPositionGenerator.get());
#else
      RMGLog::OutFormat(RMGLog::fatal, "BxDecay0 not available, please recompile remage with -DRMG_USE_BXDECAY0=ON");
#endif
      break;
    case RMGGeneratorPrimary::Generator::kUndefined :
    case RMGGeneratorPrimary::Generator::kUserDefined :
      break;
    default:
      RMGLog::Out(RMGLog::fatal, "No known implementation for generator '",
          fGenerator, "' (implement me)");
  }
  RMGLog::OutFormat(RMGLog::debug, "Primary generator set to {}", magic_enum::enum_name<RMGGeneratorPrimary::Generator>(gen));
}

void RMGGeneratorPrimary::SetConfinementCodeString(G4String code) {
  try { this->SetConfinementCode(RMGTools::ToEnum<RMGGeneratorPrimary::ConfinementCode>(code, "confinement code")); }
  catch (const std::bad_cast&) { return; }
}

void RMGGeneratorPrimary::SetGeneratorString(G4String gen) {
  try { this->SetGenerator(RMGTools::ToEnum<RMGGeneratorPrimary::Generator>(gen, "generator name")); }
  catch (const std::bad_cast&) { return; }
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
    .SetCandidates(RMGTools::GetCandidates<RMGGeneratorPrimary::ConfinementCode>())
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessenger->DeclareMethod("Select", &RMGGeneratorPrimary::SetGeneratorString)
    .SetGuidance("Select event generator")
    .SetParameterName("generator", false)
    .SetCandidates(RMGTools::GetCandidates<RMGGeneratorPrimary::Generator>())
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);
}

// vim: tabstop=2 shiftwidth=2 expandtab

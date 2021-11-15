#include "RMGMasterGenerator.hh"

#include "ProjectInfo.hh"
#include "RMGVVertexGenerator.hh"
#include "RMGVertexConfinement.hh"
#include "RMGVGenerator.hh"
#include "RMGGeneratorG4Gun.hh"
#include "RMGGeneratorGPS.hh"
#if RMG_HAS_BXDECAY0
#include "RMGGeneratorDecay0.hh"
#endif
#include "RMGLog.hh"

#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

#include "RMGTools.hh"

RMGMasterGenerator::RMGMasterGenerator():
  fConfinementCode(RMGMasterGenerator::ConfinementCode::kUnConfined),
  fGenerator(RMGMasterGenerator::Generator::kUndefined) {

  this->DefineCommands();
}

void RMGMasterGenerator::GeneratePrimaries(G4Event* event) {

  if (!fPrimaryPositionGenerator) RMGLog::Out(RMGLog::fatal, "No primary position generator (confinement) specified!");
  if (!fGeneratorObj) RMGLog::Out(RMGLog::fatal, "No primary generator specified!");

  // HACK: The BxDecay0 generator takes the responsibility for shooting the primary vertex position,
  // and this conflicts with the design I had in mind here (i.e. that a RMGVGenerator is instructed
  // about the vertex position from outside, in particular in this function here).
  if (fGenerator != RMGMasterGenerator::Generator::kBxDecay0) {
    auto vertex = G4ThreeVector();
    fPrimaryPositionGenerator->GeneratePrimariesVertex(vertex);
    RMGLog::OutDev(RMGLog::debug, "Primary vertex position: ", vertex/CLHEP::cm, " cm");
    fGeneratorObj->SetParticlePosition(vertex);
  }
  fGeneratorObj->GeneratePrimariesKinematics(event);
}

void RMGMasterGenerator::SetConfinementCode(RMGMasterGenerator::ConfinementCode code) {

  fConfinementCode = code;

  switch (fConfinementCode) {
    case ConfinementCode::kUnConfined :
      fPrimaryPositionGenerator = std::unique_ptr<RMGVVertexGenerator>(new RMGVVertexGenerator("DummyGenerator"));
      break;
    case ConfinementCode::kVolume :
      fPrimaryPositionGenerator = std::unique_ptr<RMGVertexConfinement>(new RMGVertexConfinement());
      break;
    default : RMGLog::Out(RMGLog::fatal, "No sampling strategy for confinement '",
                                         fConfinementCode, "' specified (implement me)");
  }
  RMGLog::OutFormat(RMGLog::debug, "Primary vertex confinement strategy set to {}", magic_enum::enum_name<RMGMasterGenerator::ConfinementCode>(code));
}

void RMGMasterGenerator::SetGenerator(RMGMasterGenerator::Generator gen) {

  fGenerator = gen;

  switch (fGenerator) {
    case RMGMasterGenerator::Generator::kG4gun :
      fGeneratorObj = std::make_unique<RMGGeneratorG4Gun>();
      break;
    case RMGMasterGenerator::Generator::kGPS :
      fGeneratorObj = std::make_unique<RMGGeneratorGPS>();
      break;
    case RMGMasterGenerator::Generator::kBxDecay0 :
#if RMG_HAS_BXDECAY0
      fGeneratorObj = std::make_unique<RMGGeneratorDecay0>(fPrimaryPositionGenerator.get());
#else
      RMGLog::OutFormat(RMGLog::fatal, "BxDecay0 not available, please recompile remage with -DRMG_USE_BXDECAY0=ON");
#endif
      break;
    case RMGMasterGenerator::Generator::kUndefined :
    case RMGMasterGenerator::Generator::kUserDefined :
      break;
    default:
      RMGLog::Out(RMGLog::fatal, "No known implementation for generator '",
          fGenerator, "' (implement me)");
  }
  RMGLog::OutFormat(RMGLog::debug, "Primary generator set to {}", magic_enum::enum_name<RMGMasterGenerator::Generator>(gen));
}

void RMGMasterGenerator::SetConfinementCodeString(G4String code) {
  try { this->SetConfinementCode(RMGTools::ToEnum<RMGMasterGenerator::ConfinementCode>(code, "confinement code")); }
  catch (const std::bad_cast&) { return; }
}

void RMGMasterGenerator::SetGeneratorString(G4String gen) {
  try { this->SetGenerator(RMGTools::ToEnum<RMGMasterGenerator::Generator>(gen, "generator name")); }
  catch (const std::bad_cast&) { return; }
}

void RMGMasterGenerator::SetUserGenerator(RMGVGenerator* gen) {

  fGenerator = RMGMasterGenerator::Generator::kUserDefined;
  fGeneratorObj = std::unique_ptr<RMGVGenerator>(gen);
}

void RMGMasterGenerator::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/",
      "Commands for controlling generators");

  fMessenger->DeclareMethod("Confine", &RMGMasterGenerator::SetConfinementCodeString)
    .SetGuidance("Select primary confinement strategy")
    .SetParameterName("strategy", false)
    .SetCandidates(RMGTools::GetCandidates<RMGMasterGenerator::ConfinementCode>())
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessenger->DeclareMethod("Select", &RMGMasterGenerator::SetGeneratorString)
    .SetGuidance("Select event generator")
    .SetParameterName("generator", false)
    .SetCandidates(RMGTools::GetCandidates<RMGMasterGenerator::Generator>())
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);
}

// vim: tabstop=2 shiftwidth=2 expandtab

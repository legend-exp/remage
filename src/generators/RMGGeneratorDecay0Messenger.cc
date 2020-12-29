#include "RMGGeneratorDecay0Messenger.hh"

#include "G4UIcommand.hh"

#include "RMGGeneratorDecay0.hh"
#include "RMGTools.hh"
#include "RMGLog.hh"

RMGGeneratorDecay0Messenger::RMGGeneratorDecay0Messenger(RMGGeneratorDecay0* gen) :
  fGenerator(gen) {

  G4String directory = "/RMG/Generator/Decay0";

  fDirectory = std::unique_ptr<G4UIdirectory>(new G4UIdirectory(directory));
}

void RMGGeneratorDecay0Messenger::SetNewValue(G4UIcommand* cmd, G4String new_values) {
  if (cmd == nullptr) {
  }
  else {
    RMGLog::Out(RMGLog::fatal, "Action for command '", cmd->GetTitle(), "' not implemented");
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

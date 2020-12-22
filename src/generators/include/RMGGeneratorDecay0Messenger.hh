#ifndef _RMG_GENERATOR_DECAY0_MESSENGER_HH_
#define _RMG_GENERATOR_DECAY0_MESSENGER_HH_

#include <memory>

#include "globals.hh"
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcommand;
class RMGGeneratorDecay0;
class RMGGeneratorDecay0Messenger : public G4UImessenger {

  public:

    RMGGeneratorDecay0Messenger(RMGGeneratorDecay0* gen);
    ~RMGGeneratorDecay0Messenger();

    RMGGeneratorDecay0Messenger           (RMGGeneratorDecay0Messenger const&) = delete;
    RMGGeneratorDecay0Messenger& operator=(RMGGeneratorDecay0Messenger const&) = delete;
    RMGGeneratorDecay0Messenger           (RMGGeneratorDecay0Messenger&&)      = delete;
    RMGGeneratorDecay0Messenger& operator=(RMGGeneratorDecay0Messenger&&)      = delete;

    void SetNewValue(G4UIcommand* cmd, G4String new_values) override;

  private:

    RMGGeneratorDecay0* fGenerator;

    std::unique_ptr<G4UIdirectory> fDirectory;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

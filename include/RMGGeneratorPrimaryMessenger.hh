#ifndef _RMG_GENERATOR_PRIMARY_MESSENGER_HH_
#define _RMG_GENERATOR_PRIMARY_MESSENGER_HH_

#include <memory>

#include "globals.hh"
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"

class G4UIcommand;
class RMGGeneratorPrimary;
class RMGGeneratorPrimaryMessenger : public G4UImessenger {

  public:

    RMGGeneratorPrimaryMessenger(RMGGeneratorPrimary*);
    inline ~RMGGeneratorPrimaryMessenger() = default;

    RMGGeneratorPrimaryMessenger           (RMGGeneratorPrimaryMessenger const&) = delete;
    RMGGeneratorPrimaryMessenger& operator=(RMGGeneratorPrimaryMessenger const&) = delete;
    RMGGeneratorPrimaryMessenger           (RMGGeneratorPrimaryMessenger&&)      = delete;
    RMGGeneratorPrimaryMessenger& operator=(RMGGeneratorPrimaryMessenger&&)      = delete;

    void SetNewValue(G4UIcommand* command, G4String new_values) override;

  private:

    RMGGeneratorPrimary* fGeneratorPrimary;

    std::unique_ptr<G4UIdirectory>           fGeneratorDirectory;

    std::unique_ptr<G4UIcmdWithAString>      fSelectCmd;
    std::unique_ptr<G4UIcmdWithAString>      fConfineCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

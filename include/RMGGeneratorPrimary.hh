#ifndef _RMG_GENERATOR_PRIMARY_HH_
#define _RMG_GENERATOR_PRIMARY_HH_

#include <memory>

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include "RMGVGeneratorPrimaryPosition.hh"
#include "RMGVGenerator.hh"
#include "RMGGeneratorPrimaryMessenger.hh"

class RMGGeneratorPrimary : public G4VUserPrimaryGeneratorAction {

  public:

    enum ConfinementCode {
      kUnConfined,
      kVolume
    };

    RMGGeneratorPrimary();
    ~RMGGeneratorPrimary() = default;

    RMGGeneratorPrimary           (RMGGeneratorPrimary const&) = delete;
    RMGGeneratorPrimary& operator=(RMGGeneratorPrimary const&) = delete;
    RMGGeneratorPrimary           (RMGGeneratorPrimary&&)      = delete;
    RMGGeneratorPrimary& operator=(RMGGeneratorPrimary&&)      = delete;

    void GeneratePrimaries(G4Event *event) override;

    inline RMGVGenerator* GetRMGGenerator() { return fRMGGenerator.get(); }
    inline ConfinementCode GetConfinementCode() const { return fConfinementCode; }

    void SetConfinementCode(ConfinementCode code);
    inline void SetGenerator(RMGVGenerator* gen) { fRMGGenerator = std::unique_ptr<RMGVGenerator>(gen); }

  private:

    ConfinementCode fConfinementCode;
    std::unique_ptr<RMGVGeneratorPrimaryPosition>  fPrimaryPositionGenerator;
    std::unique_ptr<RMGVGenerator>                 fRMGGenerator;
    std::unique_ptr<RMGGeneratorPrimaryMessenger>  fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

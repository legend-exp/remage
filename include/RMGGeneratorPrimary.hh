#ifndef _RMG_GENERATOR_PRIMARY_HH_
#define _RMG_GENERATOR_PRIMARY_HH_

#include <memory>

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include "RMGVGeneratorPrimaryPosition.hh"
#include "RMGVGenerator.hh"

class G4GenericMessenger;
class RMGGeneratorPrimary : public G4VUserPrimaryGeneratorAction {

  public:

    enum ConfinementCode {
      kUnConfined,
      kVolume
    };

    enum Generator {
      kG4gun,
      kSPS,
      kBxDecay0,
      kUserDefined,
      kUndefined
    };

    RMGGeneratorPrimary();
    ~RMGGeneratorPrimary() = default;

    RMGGeneratorPrimary           (RMGGeneratorPrimary const&) = delete;
    RMGGeneratorPrimary& operator=(RMGGeneratorPrimary const&) = delete;
    RMGGeneratorPrimary           (RMGGeneratorPrimary&&)      = delete;
    RMGGeneratorPrimary& operator=(RMGGeneratorPrimary&&)      = delete;

    void GeneratePrimaries(G4Event *event) override;

    inline RMGVGenerator* GetGenerator() { return fGeneratorObj.get(); }
    inline ConfinementCode GetConfinementCode() const { return fConfinementCode; }

    void SetConfinementCode(ConfinementCode code);
    void SetConfinementCodeString(G4String code);
    void SetUserGenerator(RMGVGenerator* gen);
    void SetGenerator(Generator gen);
    void SetGeneratorString(G4String gen);

  private:

    ConfinementCode fConfinementCode;
    std::unique_ptr<RMGVGeneratorPrimaryPosition> fPrimaryPositionGenerator;

    Generator fGenerator;
    std::unique_ptr<RMGVGenerator> fGeneratorObj;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

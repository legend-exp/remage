#ifndef _RMG_GENERATOR_PRIMARY_HH_
#define _RMG_GENERATOR_PRIMARY_HH_

#include <memory>

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include "RMGVVertexGenerator.hh"
#include "RMGVGenerator.hh"

class G4GenericMessenger;
class RMGMasterGenerator : public G4VUserPrimaryGeneratorAction {

  public:

    enum ConfinementCode {
      kUnConfined,
      kVolume
    };

    enum Generator {
      kG4gun,
      kGPS,
      kBxDecay0,
      kUserDefined,
      kUndefined
    };

    RMGMasterGenerator();
    ~RMGMasterGenerator() = default;

    RMGMasterGenerator           (RMGMasterGenerator const&) = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator const&) = delete;
    RMGMasterGenerator           (RMGMasterGenerator&&)      = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator&&)      = delete;

    void GeneratePrimaries(G4Event *event) override;

    inline RMGVGenerator* GetGenerator() { return fGeneratorObj.get(); }
    inline ConfinementCode GetConfinementCode() const { return fConfinementCode; }

    void SetConfinementCode(ConfinementCode code);
    void SetConfinementCodeString(std::string code);
    void SetUserGenerator(RMGVGenerator* gen);
    void SetGenerator(Generator gen);
    void SetGeneratorString(std::string gen);

  private:

    ConfinementCode fConfinementCode;
    std::unique_ptr<RMGVVertexGenerator> fPrimaryPositionGenerator;

    Generator fGenerator;
    std::unique_ptr<RMGVGenerator> fGeneratorObj;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _RMG_GENERATOR_PRIMARY_HH_
#define _RMG_GENERATOR_PRIMARY_HH_

#include <memory>

#include "G4VUserPrimaryGeneratorAction.hh"

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

class G4GenericMessenger;
class RMGMasterGenerator : public G4VUserPrimaryGeneratorAction {

  public:

    enum Confinement {
      kUnConfined,
      kVolume,
      kFromFile,
    };

    enum Generator {
      kG4gun,
      kGPS,
      kBxDecay0,
      kCosmicMuons,
      kUserDefined,
      kUndefined
    };

    RMGMasterGenerator();
    ~RMGMasterGenerator() = default;

    RMGMasterGenerator(RMGMasterGenerator const&) = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator const&) = delete;
    RMGMasterGenerator(RMGMasterGenerator&&) = delete;
    RMGMasterGenerator& operator=(RMGMasterGenerator&&) = delete;

    void GeneratePrimaries(G4Event* event) override;

    inline RMGVGenerator* GetGenerator() { return fGeneratorObj.get(); }
    inline Confinement GetConfinement() const { return fConfinement; }

    void SetConfinement(Confinement code);
    void SetConfinementString(std::string code);
    void SetUserGenerator(RMGVGenerator* gen);
    void SetGenerator(Generator gen);
    void SetGeneratorString(std::string gen);

  private:

    Confinement fConfinement;
    std::unique_ptr<RMGVVertexGenerator> fVertexGenerator;

    Generator fGenerator;
    std::unique_ptr<RMGVGenerator> fGeneratorObj;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

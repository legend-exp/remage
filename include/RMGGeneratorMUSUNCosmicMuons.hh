#ifndef _RMG_GENERATOR_MUSUN_COSMIC_MUONS_HH_
#define _RMG_GENERATOR_MUSUN_COSMIC_MUONS_HH_

#include "RMGVGenerator.hh"
#include "RMGVVertexGenerator.hh"

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include "G4CsvAnalysisReader.hh"
#include <filesystem>

namespace u = CLHEP;

struct RMGGeneratorMUSUNCosmicMuons_Data {
    G4int fID;
    G4int fType;
    G4double fEkin;
    G4double fX;
    G4double fY;
    G4double fZ;
    G4double fTheta;
    G4double fPhi;
    G4double fPx;
    G4double fPy;
    G4double fPz;
};


class G4Event;
class RMGGeneratorMUSUNCosmicMuons : public RMGVGenerator {

  public:

    RMGGeneratorMUSUNCosmicMuons();
    ~RMGGeneratorMUSUNCosmicMuons() = default;

    RMGGeneratorMUSUNCosmicMuons(RMGGeneratorMUSUNCosmicMuons const&) = delete;
    RMGGeneratorMUSUNCosmicMuons& operator=(RMGGeneratorMUSUNCosmicMuons const&) = delete;
    RMGGeneratorMUSUNCosmicMuons(RMGGeneratorMUSUNCosmicMuons&&) = delete;
    RMGGeneratorMUSUNCosmicMuons& operator=(RMGGeneratorMUSUNCosmicMuons&&) = delete;

    void GeneratePrimaries(G4Event* event);
    virtual void SetParticlePosition(G4ThreeVector vec) override {};


    void BeginOfRunAction(const G4Run*);
    void EndOfRunAction(const G4Run*);

  private:

    void DefineCommands();
    void SetMUSUNFile(G4String pathToFile);
    void PrepareCopy(G4String pathToFile);

    std::unique_ptr<G4ParticleGun> fGun = nullptr;
    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    G4String fPathToFile = "";
    std::filesystem::path fPathToTmpFolder;
    G4String fPathToTmpFile = "";

    static G4CsvAnalysisReader* fAnalysisReader;

    static RMGGeneratorMUSUNCosmicMuons_Data* input_data;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

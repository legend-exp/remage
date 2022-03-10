#ifndef _RMG_GENERATOR_COSMIC_MUONS_HH_
#define _RMG_GENERATOR_COSMIC_MUONS_HH_

#include <memory>
#include <string>

#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "CLHEP/Units/SystemOfUnits.h"

#include "EcoMug/EcoMug.h"

namespace u = CLHEP;

// TODO: must inherit from RMGVGenerator
class RMGGeneratorCosmicMuons : public G4VUserPrimaryGeneratorAction {

  public:

    enum SkyShape {
      kPlane,
      kSphere
    };

    RMGGeneratorCosmicMuons();
    ~RMGGeneratorCosmicMuons() = default;

    RMGGeneratorCosmicMuons           (RMGGeneratorCosmicMuons const&) = delete;
    RMGGeneratorCosmicMuons& operator=(RMGGeneratorCosmicMuons const&) = delete;
    RMGGeneratorCosmicMuons           (RMGGeneratorCosmicMuons&&)      = delete;
    RMGGeneratorCosmicMuons& operator=(RMGGeneratorCosmicMuons&&)      = delete;

    void GeneratePrimaries(G4Event *event) override;
    void BeginOfRunAction();
    inline void EndOfRunAction() {}

  private:

    std::unique_ptr<EcoMug> fEcoMug = nullptr;
    std::unique_ptr<G4ParticleGun> fGun = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();
    void SetSkyShape(std::string shape);

    SkyShape fSkyShape = kSphere;
    float fSkyPlaneSize = -1;

    float fSpherePositionThetaMin = 0 * u::deg;
    float fSpherePositionThetaMax = 90 * u::deg;
    float fSpherePositionPhiMin   = 0 * u::deg;
    float fSpherePositionPhiMax   = 360 * u::deg;

    float fMomentumMin = 0 * u::GeV;
    float fMomentumMax = 1 * u::TeV;
    float fThetaMin    = 0 * u::deg;
    float fThetaMax    = 90 * u::deg;
    float fPhiMin      = 0 * u::deg;
    float fPhiMax      = 360 * u::deg;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

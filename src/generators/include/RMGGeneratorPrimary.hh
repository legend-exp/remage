#ifndef _RMGGENERATORPRIMARY_HH
#define _RMGGENERATORPRIMARY_HH

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

class RMGVGeneratorPrimaryPosition;
class RMGVGenerator;
class RMGGeneratorPrimaryMessenger;
class RMGGeneratorPrimary : public G4VUserPrimaryGeneratorAction {

  public:

    enum ConfinementCode {
      kUnConfined,
      kVolume
    };

    RMGGeneratorPrimary();
    ~RMGGeneratorPrimary();

    RMGGeneratorPrimary           (RMGGeneratorPrimary const&) = delete;
    RMGGeneratorPrimary& operator=(RMGGeneratorPrimary const&) = delete;
    RMGGeneratorPrimary           (RMGGeneratorPrimary&&)      = delete;
    RMGGeneratorPrimary& operator=(RMGGeneratorPrimary&&)      = delete;

    void GeneratePrimaries(G4Event *event) override;

    inline RMGVGenerator*  GetRMGGenerator() { return fRMGGenerator; }
    inline ConfinementCode GetConfinementCode() { return fConfinementCode; }

    void SetConfinementCode(ConfinementCode code);
    inline void SetGenerator(RMGVGenerator* gen) { fRMGGenerator = gen; }

  private:

    ConfinementCode                fConfinementCode;
    RMGVGeneratorPrimaryPosition*  fPrimaryPositionGenerator;
    RMGVGenerator*                 fRMGGenerator;
    RMGGeneratorPrimaryMessenger*  fG4Messenger;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

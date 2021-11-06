#ifndef _RMGVGENERATOR_HH_
#define _RMGVGENERATOR_HH_

#include <memory>

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"

class G4Event;
class G4Run;
class RMGVGenerator {

  public:

    RMGVGenerator() = delete;

    inline RMGVGenerator(G4String name) :
      fGeneratorName(name),
      fReportingFrequency(1000) {};

    virtual inline ~RMGVGenerator() = default;

    RMGVGenerator           (RMGVGenerator const&) = delete;
    RMGVGenerator& operator=(RMGVGenerator const&) = delete;
    RMGVGenerator           (RMGVGenerator&&)      = delete;
    RMGVGenerator& operator=(RMGVGenerator&&)      = delete;

    virtual inline void BeginOfRunAction(const G4Run*) {};
    virtual inline void EndOfRunAction(const G4Run*) {};

    virtual void SetParticlePosition(G4ThreeVector vec) = 0;
    virtual void GeneratePrimariesKinematics(G4Event*) = 0;

    inline void SetReportingFrequency(G4int freq) { fReportingFrequency = freq; }
    inline G4String GetGeneratorName() { return fGeneratorName; }

  protected:

    G4String fGeneratorName;
    std::unique_ptr<G4UImessenger> fG4Messenger;
    G4int fReportingFrequency;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

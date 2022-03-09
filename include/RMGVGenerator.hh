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

    inline RMGVGenerator(std::string name) :
      fGeneratorName(name) {};

    virtual inline ~RMGVGenerator() = default;

    RMGVGenerator           (RMGVGenerator const&) = delete;
    RMGVGenerator& operator=(RMGVGenerator const&) = delete;
    RMGVGenerator           (RMGVGenerator&&)      = delete;
    RMGVGenerator& operator=(RMGVGenerator&&)      = delete;

    virtual inline void BeginOfRunAction(const G4Run*) {};
    virtual inline void EndOfRunAction(const G4Run*) {};

    virtual void SetParticlePosition(G4ThreeVector vec) = 0;
    virtual void GeneratePrimariesKinematics(G4Event*) = 0;

    inline void SetReportingFrequency(int freq) { fReportingFrequency = freq; }
    inline std::string GetGeneratorName() { return fGeneratorName; }

  protected:

    std::string fGeneratorName;
    std::unique_ptr<G4UImessenger> fG4Messenger;
    int fReportingFrequency = 1000;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

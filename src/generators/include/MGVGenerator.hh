#ifndef _MGVGENERATOR_HH
#define _MGVGENERATOR_HH

#include "globals.hh"
#include "G4ThreeVector.hh"

class G4Event;
class G4UImessenger;
class G4Run;
class MGVGenerator {

  public:

    inline MGVGenerator() :
      fGeneratorName(""),
      fG4Messenger(0),
      fReportingFrequency(1000) {};

    virtual ~MGVGenerator() = default;
    MGVGenerator           (MGVGenerator const&) = delete;
    MGVGenerator& operator=(MGVGenerator const&) = delete;
    MGVGenerator           (MGVGenerator&&)      = delete;
    MGVGenerator& operator=(MGVGenerator&&)      = delete;

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);
    virtual void GeneratePrimaryVertex(G4Event *event);
    virtual void SetParticlePosition(G4ThreeVector vec);
    inline G4String GetGeneratorName() { return fGeneratorName; }
    inline void SetReportingFrequency(G4int freq) { fReportingFrequency = freq; }

  protected:

    G4String      fGeneratorName;      ///< Name of Generator
    G4UImessenger *fG4Messenger;       ///< G4Messenger for setting up generator
    G4int         fReportingFrequency; ///< Report generation rate
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

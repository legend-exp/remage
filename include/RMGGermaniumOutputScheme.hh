#ifndef _RMG_GERMANIUM_OUTPUT_SCHEME_HH_
#define _RMG_GERMANIUM_OUTPUT_SCHEME_HH_

#include <vector>

#include "RMGVOutputScheme.hh"

class G4Event;
class RMGGermaniumOutputScheme : public RMGVOutputScheme {

  public:

    inline RMGGermaniumOutputScheme(G4AnalysisManager* ana_man) : RMGVOutputScheme(ana_man) {}

    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void EndOfEventAction(const G4Event*) override;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

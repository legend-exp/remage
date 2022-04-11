#ifndef _RMG_OPTICAL_OUTPUT_SCHEME_HH_
#define _RMG_OPTICAL_OUTPUT_SCHEME_HH_

#include <vector>

#include "RMGVOutputScheme.hh"

class G4Event;
class RMGOpticalOutputScheme : public RMGVOutputScheme {

  public:

    inline RMGOpticalOutputScheme(G4AnalysisManager* ana_man) : RMGVOutputScheme(ana_man) {}

    void clear() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void EndOfEventAction(const G4Event*) override;

  private:

    std::vector<int> detector_uid;
    std::vector<int> photoelectrons;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _ISOTOPE_OUTPUT_SCHEME_HH_
#define _ISOTOPE_OUTPUT_SCHEME_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

class G4Event;
class IsotopeOutputScheme : public RMGVOutputScheme {

  public:

    IsotopeOutputScheme() = default;

    void ClearBeforeEvent() override;
    void AssignOutputNames(G4AnalysisManager* ana_man) override;
    void StoreEvent(const G4Event*) override;
    // bool ShouldDiscardEvent(const G4Event*) override;
    // std::optional<bool> StackingActionNewStage(int) override;
    // std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    void TrackingActionPre(const G4Track* aTrack) override;

  private:

    // Make sure the ID will not end up being used by other detectors (should be unique)
    G4int OutputRegisterID = 1000;

    std::vector<G4int> zOfEvent;
    std::vector<G4int> aOfEvent;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

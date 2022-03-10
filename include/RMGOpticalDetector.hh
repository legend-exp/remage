#ifndef _MUG_OPTICAL_SENSITIVE_DETECTOR_HH_
#define _MUG_OPTICAL_SENSITIVE_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4VSensitiveDetector.hh"

#include "RMGOpticalDetectorHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
class G4GenericMessenger;
class RMGOpticalDetector : public G4VSensitiveDetector {

  public:

    RMGOpticalDetector(const std::string& name, const std::string& hits_coll_name);
    ~RMGOpticalDetector() = default;

    RMGOpticalDetector           (RMGOpticalDetector const&) = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector const&) = delete;
    RMGOpticalDetector           (RMGOpticalDetector&&)      = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector&&)      = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGOpticalDetectorHitsCollection* fHitsCollection = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef _MUG_OPTICAL_DETECTOR_HH_
#define _MUG_OPTICAL_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"

class RMGOpticalDetectorHit : public G4VHit {

  public:

    RMGOpticalDetectorHit() = default;
    ~RMGOpticalDetectorHit() = default;

    RMGOpticalDetectorHit(RMGOpticalDetectorHit const&) = delete;
    RMGOpticalDetectorHit& operator=(RMGOpticalDetectorHit const&) = delete;
    RMGOpticalDetectorHit(RMGOpticalDetectorHit&&) = delete;
    RMGOpticalDetectorHit& operator=(RMGOpticalDetectorHit&&) = delete;

    bool operator==(const RMGOpticalDetectorHit&) const;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    // getters
    inline int GetDetectorUID() { return fDetectorUID; }
    inline float GetPhotonEnergy() { return fPhotonEnergy; }

    // setters
    inline void SetDetectorUID(int id) { fDetectorUID = id; }
    inline void SetPhotonEnergy(float energy) { fPhotonEnergy = energy; }

    void Print() override;
    // TODO
    // void Draw() override;

  private:

    int fDetectorUID = -1;
    float fPhotonEnergy = 0.;
};

typedef G4THitsCollection<RMGOpticalDetectorHit> RMGOpticalDetectorHitsCollection;

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
class G4GenericMessenger;
class RMGOpticalDetector : public G4VSensitiveDetector {

  public:

    RMGOpticalDetector();
    ~RMGOpticalDetector() = default;

    RMGOpticalDetector(RMGOpticalDetector const&) = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector const&) = delete;
    RMGOpticalDetector(RMGOpticalDetector&&) = delete;
    RMGOpticalDetector& operator=(RMGOpticalDetector&&) = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGOpticalDetectorHitsCollection* fHitsCollection = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();
};

extern G4ThreadLocal G4Allocator<RMGOpticalDetectorHit>* RMGOpticalDetectorHitAllocator;

inline void* RMGOpticalDetectorHit::operator new(size_t) {
  if (!RMGOpticalDetectorHitAllocator)
    RMGOpticalDetectorHitAllocator = new G4Allocator<RMGOpticalDetectorHit>;
  return (void*)RMGOpticalDetectorHitAllocator->MallocSingle();
}

inline void RMGOpticalDetectorHit::operator delete(void* hit) {
  RMGOpticalDetectorHitAllocator->FreeSingle((RMGOpticalDetectorHit*)hit);
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

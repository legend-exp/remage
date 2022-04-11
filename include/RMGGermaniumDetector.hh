#ifndef _MUG_GERMANIUM_DETECTOR_HH_
#define _MUG_GERMANIUM_DETECTOR_HH_

#include <memory>
#include <string>

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4VSensitiveDetector.hh"

class RMGGermaniumDetectorHit : public G4VHit {

  public:

    RMGGermaniumDetectorHit() = default;
    ~RMGGermaniumDetectorHit() = default;

    RMGGermaniumDetectorHit           (RMGGermaniumDetectorHit const&) = delete;
    RMGGermaniumDetectorHit& operator=(RMGGermaniumDetectorHit const&) = delete;
    RMGGermaniumDetectorHit           (RMGGermaniumDetectorHit&&)      = delete;
    RMGGermaniumDetectorHit& operator=(RMGGermaniumDetectorHit&&)      = delete;

    bool operator==(const RMGGermaniumDetectorHit&) const;

    inline void* operator new(size_t);
    inline void  operator delete(void*);

    // getters
    inline int GetDetectorUID() { return fDetectorUID; }
    inline int GetPhotoelectrons() { return fPhotoElectrons; }

    // setters
    inline void SetDetectorUID(int id) { fDetectorUID = id; }
    inline void AddPhotoElectron() { fPhotoElectrons++; }

    void Print() override;
    // TODO
    // void Draw() override;

  private:

    int fDetectorUID = -1;
    int fPhotoElectrons = 0;
};

typedef G4THitsCollection<RMGGermaniumDetectorHit> RMGGermaniumDetectorHitsCollection;

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;
class G4GenericMessenger;
class RMGGermaniumDetector : public G4VSensitiveDetector {

  public:

    RMGGermaniumDetector();
    ~RMGGermaniumDetector() = default;

    RMGGermaniumDetector           (RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector const&) = delete;
    RMGGermaniumDetector           (RMGGermaniumDetector&&)      = delete;
    RMGGermaniumDetector& operator=(RMGGermaniumDetector&&)      = delete;

    void Initialize(G4HCofThisEvent* hit_coll) override;
    bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hit_coll) override;

  private:

    RMGGermaniumDetectorHitsCollection* fHitsCollection = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();
};

extern G4ThreadLocal G4Allocator<RMGGermaniumDetectorHit>* RMGGermaniumDetectorHitAllocator;

inline void* RMGGermaniumDetectorHit::operator new(size_t) {
  if(!RMGGermaniumDetectorHitAllocator)
      RMGGermaniumDetectorHitAllocator = new G4Allocator<RMGGermaniumDetectorHit>;
  return (void *) RMGGermaniumDetectorHitAllocator->MallocSingle();
}

inline void RMGGermaniumDetectorHit::operator delete(void *hit) {
  RMGGermaniumDetectorHitAllocator->FreeSingle((RMGGermaniumDetectorHit*) hit);
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

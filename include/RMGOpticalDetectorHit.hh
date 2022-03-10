#ifndef _RMG_OPTICAL_DETECTOR_HIT_HH_
#define _RMG_OPTICAL_DETECTOR_HIT_HH_

#include <memory>

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class RMGOpticalDetectorHit : public G4VHit {

  public:

    RMGOpticalDetectorHit() = default;
    ~RMGOpticalDetectorHit() = default;

    RMGOpticalDetectorHit           (RMGOpticalDetectorHit const&) = delete;
    RMGOpticalDetectorHit& operator=(RMGOpticalDetectorHit const&) = delete;
    RMGOpticalDetectorHit           (RMGOpticalDetectorHit&&)      = delete;
    RMGOpticalDetectorHit& operator=(RMGOpticalDetectorHit&&)      = delete;

    bool operator==(const RMGOpticalDetectorHit&) const;

    inline void* operator new(size_t);
    inline void  operator delete(void*);

    // getters
    inline int GetDetectorID() { return fDetectorID; }

    // setters
    inline void SetDetectorID(int id) { fDetectorID = id; }

    // TODO: override Draw
    void Print() override;

  private:

    int fDetectorID = -1;
};

typedef G4THitsCollection<RMGOpticalDetectorHit> RMGOpticalDetectorHitsCollection;

extern G4ThreadLocal G4Allocator<RMGOpticalDetectorHit>* RMGOpticalDetectorHitAllocator;

inline void* RMGOpticalDetectorHit::operator new(size_t) {
  if(!RMGOpticalDetectorHitAllocator)
      RMGOpticalDetectorHitAllocator = new G4Allocator<RMGOpticalDetectorHit>;
  return (void *) RMGOpticalDetectorHitAllocator->MallocSingle();
}

inline void RMGOpticalDetectorHit::operator delete(void *hit) {
  RMGOpticalDetectorHitAllocator->FreeSingle((RMGOpticalDetectorHit*) hit);
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

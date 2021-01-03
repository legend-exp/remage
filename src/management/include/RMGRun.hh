#ifndef _RMG_RUN_HH_
#define _RMG_RUN_HH_

#include <chrono>

#include "G4Run.hh"

class RMGRun : public G4Run {

  public:

    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    inline const TimePoint& GetStartTime() const { return fStartTime; }
    inline void SetStartTime(TimePoint t) { fStartTime = t; }

  private:

    TimePoint fStartTime;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

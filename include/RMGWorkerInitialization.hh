// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _RMG_WORKER_INITIALIZATION_HH_
#define _RMG_WORKER_INITIALIZATION_HH_

#include <type_traits>

#include "G4AutoLock.hh"
#include "G4UserTaskThreadInitialization.hh"
#include "Randomize.hh"

#include "RMGManager.hh"

/// \cond this creates weird namespaces @<long number>
namespace {
  G4Mutex RMGWorkerInitializationRNGMutex = G4MUTEX_INITIALIZER;
} // namespace
/// \endcond

template<typename ThreadInitialization>
class RMGWorkerInitialization : public ThreadInitialization {
    static_assert(std::is_base_of<G4UserWorkerThreadInitialization, ThreadInitialization>::value);

  public:

    RMGWorkerInitialization() = default;
    ~RMGWorkerInitialization() = default;

    inline void SetupRNGEngine(const CLHEP::HepRandomEngine* aRNGEngine) const override {
      G4AutoLock l(&RMGWorkerInitializationRNGMutex);

      auto rmg_man = RMGManager::Instance();
      // Implement custom logic if the user had chosen a random engine, otherwise apply Geant4's
      // default strategy.
      if (rmg_man->GetRandEngineSelected()) rmg_man->ApplyRandEngineForCurrentThread();
      else ThreadInitialization::SetupRNGEngine(aRNGEngine);
    }
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

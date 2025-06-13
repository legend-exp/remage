// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#ifndef _RMG_USER_INIT_HH_
#define _RMG_USER_INIT_HH_

#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "G4UserSteppingAction.hh"
#include "G4UserTrackingAction.hh"

#include "RMGIsotopeFilterScheme.hh"
#include "RMGLog.hh"
#include "RMGParticleFilterScheme.hh"
#include "RMGTrackOutputScheme.hh"
#include "RMGVGenerator.hh"
#include "RMGVOutputScheme.hh"

class RMGUserInit {

  public:

    RMGUserInit() = default;
    ~RMGUserInit() = default;

    RMGUserInit(RMGUserInit const&) = delete;
    RMGUserInit& operator=(RMGUserInit const&) = delete;
    RMGUserInit(RMGUserInit&&) = delete;
    RMGUserInit& operator=(RMGUserInit&&) = delete;

    // getters
    [[nodiscard]] auto GetSteppingActions() const { return fSteppingActions; }
    [[nodiscard]] auto GetTrackingActions() const { return fTrackingActions; }
    [[nodiscard]] auto GetOutputSchemes() const { return fOutputSchemes; }
    [[nodiscard]] auto GetOptionalOutputSchemes() const { return fOptionalOutputSchemes; }
    [[nodiscard]] auto GetUserGenerator() const { return fUserGenerator; }

    // setters
    template<typename T, typename... Args> void AddSteppingAction(Args&&... args) {
      Add<T>(&fSteppingActions, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args> void AddTrackingAction(Args&&... args) {
      Add<T>(&fTrackingActions, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args> void AddOutputScheme(Args&&... args) {
      Add<T>(&fOutputSchemes, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    void AddOptionalOutputScheme(std::string name, Args&&... args) {
      Add<T>(&fOptionalOutputSchemes, name, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args> void SetUserGenerator(Args&&... args) {
      Set<T>(fUserGenerator, std::forward<Args>(args)...);
    }

    // default output schemes
    void RegisterDefaultOptionalOutputSchemes() {
      AddOptionalOutputScheme<RMGIsotopeFilterScheme>("IsotopeFilter");
      AddOptionalOutputScheme<RMGParticleFilterScheme>("ParticleFilter");
      AddOptionalOutputScheme<RMGTrackOutputScheme>("Track");
    }

    void ActivateOptionalOutputScheme(std::string name) {
      auto it = fOptionalOutputSchemes.find(name);
      if (it == fOptionalOutputSchemes.end()) {
        RMGLog::Out(RMGLog::fatal, "Optional output scheme '", name, "' not found!");
      }
      fOutputSchemes.emplace_back((*it).second);
    }

  private:

    template<typename T> using late_init_fn = std::function<std::unique_ptr<T>()>;
    template<typename T> using late_init_vec = std::vector<late_init_fn<T>>;
    template<typename K, typename T> using late_init_map = std::map<K, late_init_fn<T>>;

    template<typename B, typename T, typename... Args> late_init_fn<B> CreateInit(Args&&... args) {
      return std::bind(&std::make_unique<T, Args&...>, std::forward<Args>(args)...);
    }
    template<typename B, typename T> late_init_fn<B> CreateInit() {
      return [] { return std::make_unique<T>(); };
    }

    template<typename T, typename B, typename... Args>
    void Add(late_init_vec<B>* vec, Args&&... args) {
      static_assert(std::is_base_of<B, T>::value);

      // capture the passed arguments for the constructor to be called later.
      auto create = CreateInit<B, T>(std::forward<Args>(args)...);
      vec->emplace_back(std::move(create));
    }
    template<typename T, typename B, typename K, typename... Args>
    void Add(late_init_map<K, B>* map, K k, Args&&... args) {
      static_assert(std::is_base_of<B, T>::value);

      // capture the passed arguments for the constructor to be called later.
      auto create = CreateInit<B, T>(std::forward<Args>(args)...);
      map->emplace(k, std::move(create));
    }
    template<typename T, typename B, typename... Args>
    void Set(late_init_fn<B>& fn, Args&&... args) {
      static_assert(std::is_base_of<B, T>::value);

      // capture the passed arguments for the constructor to be called later.
      auto create = CreateInit<B, T>(std::forward<Args>(args)...);
      fn.swap(create);
    }

    // store partial custom actions to be used later in RMGUserAction
    late_init_vec<G4UserSteppingAction> fSteppingActions;
    late_init_vec<G4UserTrackingAction> fTrackingActions;
    late_init_vec<RMGVOutputScheme> fOutputSchemes;
    late_init_map<std::string, RMGVOutputScheme> fOptionalOutputSchemes;
    late_init_fn<RMGVGenerator> fUserGenerator;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

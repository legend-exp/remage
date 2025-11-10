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

#include "RMGGeometryCheckOutputScheme.hh"
#include "RMGIsotopeFilterScheme.hh"
#include "RMGLog.hh"
#include "RMGParticleFilterScheme.hh"
#include "RMGTrackOutputScheme.hh"
#include "RMGVGenerator.hh"
#include "RMGVOutputScheme.hh"
#include "RMGBenchmarkOutputScheme.hh"

/**
 * @brief User initialization class.
 *
 * This class manages user-defined actions, output schemes, and generator configuration
 * for the remage simulation. It provides methods to add stepping actions, tracking actions,
 * output schemes, and to configure the user generator.
 *
 * @details This class does not manage instances, but stores arguments that will be forwarded
 * several times to initialize the user-defined actions on all worker threads.
 */
class RMGUserInit {

  public:

    /**
     * @brief Default constructor.
     */
    RMGUserInit() = default;
    /**
     * @brief Default destructor.
     */
    ~RMGUserInit() = default;

    RMGUserInit(RMGUserInit const&) = delete;
    RMGUserInit& operator=(RMGUserInit const&) = delete;
    RMGUserInit(RMGUserInit&&) = delete;
    RMGUserInit& operator=(RMGUserInit&&) = delete;

    // getters
    /**
     * @brief Retrieves the collection of stepping actions.
     * @return A vector of functions generating G4UserSteppingAction objects.
     */
    [[nodiscard]] auto GetSteppingActions() const { return fSteppingActions; }
    /**
     * @brief Retrieves the collection of tracking actions.
     * @return A vector of functions generating G4UserTrackingAction objects.
     */
    [[nodiscard]] auto GetTrackingActions() const { return fTrackingActions; }
    /**
     * @brief Retrieves the registered output schemes.
     * @return A vector of functions generating RMGVOutputScheme objects.
     */
    [[nodiscard]] auto GetOutputSchemes() const { return fOutputSchemes; }
    /**
     * @brief Retrieves the optional output schemes.
     * @return A map of functions generating RMGVOutputScheme objects, keyed by scheme name.
     */
    [[nodiscard]] auto GetOptionalOutputSchemes() const { return fOptionalOutputSchemes; }
    /**
     * @brief Retrieves the user generator.
     * @return A function generating an RMGVGenerator object.
     */
    [[nodiscard]] auto GetUserGenerator() const { return fUserGenerator; }

    // setters
    /**
     * @brief Adds a stepping action of type T.
     *
     * @tparam T Derived type of G4UserSteppingAction.
     * @param args Arguments to forward to T's constructor.
     */
    template<typename T, typename... Args> void AddSteppingAction(Args&&... args) {
      Add<T>(&fSteppingActions, std::forward<Args>(args)...);
    }

    /**
     * @brief Adds a tracking action of type T.
     *
     * @tparam T Derived type of G4UserTrackingAction.
     * @param args Arguments to forward to T's constructor.
     */
    template<typename T, typename... Args> void AddTrackingAction(Args&&... args) {
      Add<T>(&fTrackingActions, std::forward<Args>(args)...);
    }

    /**
     * @brief Adds an output scheme of type T.
     *
     * @tparam T Derived type of RMGVOutputScheme.
     * @param args Arguments to forward to T's constructor.
     */
    template<typename T, typename... Args> void AddOutputScheme(Args&&... args) {
      Add<T>(&fOutputSchemes, std::forward<Args>(args)...);
    }

    /**
     * @brief Adds an optional output scheme of type T with a given name.
     * @details These additional output schemes can be activated with
     * @ref RMGManager::ActivateOptionalOutputScheme.
     *
     * @tparam T Derived type of RMGVOutputScheme.
     * @param name The key under which the scheme will be stored.
     * @param args Arguments to forward to T's constructor.
     */
    template<typename T, typename... Args>
    void AddOptionalOutputScheme(std::string name, Args&&... args) {
      Add<T>(&fOptionalOutputSchemes, name, std::forward<Args>(args)...);
    }

    /**
     * @brief Sets the user generator to an instance of type T.
     *
     * @tparam T Derived type of RMGVGenerator.
     * @param args Arguments to forward to T's constructor.
     */
    template<typename T, typename... Args> void SetUserGenerator(Args&&... args) {
      Set<T>(fUserGenerator, std::forward<Args>(args)...);
    }

    // default output schemes
    /**
     * @brief Registers the default optional output schemes.
     *
     * This includes schemes for isotope filtering, particle filtering, and track output.
     */
    void RegisterDefaultOptionalOutputSchemes() {
      AddOptionalOutputScheme<RMGIsotopeFilterScheme>("IsotopeFilter");
      AddOptionalOutputScheme<RMGParticleFilterScheme>("ParticleFilter");
      AddOptionalOutputScheme<RMGTrackOutputScheme>("Track");
      AddOptionalOutputScheme<RMGGeometryCheckOutputScheme>("GeometryCheck");
      AddOptionalOutputScheme<RMGBenchmarkOutputScheme>("Benchmark");
      
    }

    /**
     * @brief Activates an optional output scheme by its name.
     *
     * If the scheme is not found, a fatal error is logged.
     *
     * @param name The name of the optional output scheme to activate.
     */
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

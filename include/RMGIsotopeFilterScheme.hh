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

#ifndef _RMG_ISOTOPE_FILTER_SCHEME_HH_
#define _RMG_ISOTOPE_FILTER_SCHEME_HH_

#include <optional>
#include <set>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4VUserEventInformation.hh"

#include "RMGVOutputScheme.hh"

/**
 * @brief Marker attached to an event when one of the requested isotopes has been produced.
 *
 * Used internally by @ref RMGIsotopeFilterScheme to communicate detection of a target
 * isotope between the tracking and event actions; carries no payload.
 */
class RMGIsotopeFilterEventInformation : public G4VUserEventInformation {

  public:

    RMGIsotopeFilterEventInformation() = default;
    void Print() const override {}
};

class G4Event;
/**
 * @brief Output filter discarding events that did not produce any of a given isotope list.
 *
 * Tracks created in the event are inspected; if no track corresponds to a @c (A, Z) pair
 * in the configured isotope set, the event is discarded. With
 * @c fDiscardPhotonsIfIsotopeNotProduced enabled, photon (and optical) tracks are deferred
 * to a second stacking stage so that they are only tracked if a matching isotope is
 * actually produced first, saving CPU on rejected events.
 */
class RMGIsotopeFilterScheme : public RMGVOutputScheme {

  public:

    RMGIsotopeFilterScheme();

    /** @brief Discard the event if none of the registered isotopes was produced. */
    bool ShouldDiscardEvent(const G4Event*) override;
    /** @brief Drive the two-stage stacking when photon deferral is enabled. */
    std::optional<bool> StackingActionNewStage(int stage) override;
    /** @brief Defer photons to stage 1; otherwise leave the classification unchanged. */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int stage) override;
    /** @brief Tag the current event if the track's particle matches a registered isotope. */
    void TrackingActionPre(const G4Track*) override;

    /** @brief Add an @c (A, Z) isotope to the keep list. */
    void AddIsotope(int a, int z) { fIsotopes.insert({a, z}); }

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    std::set<std::pair<int, int>> fIsotopes;

    bool fDiscardPhotonsIfIsotopeNotProduced = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

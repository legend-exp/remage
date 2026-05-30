// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

#ifndef _RMG_TRACKING_ACTION_HH_
#define _RMG_TRACKING_ACTION_HH_

#include <memory>

#include "G4GenericMessenger.hh"
#include "G4UserTrackingAction.hh"

class RMGRunAction;
/**
 * @brief Tracking action delegating to output schemes and guarding global-time precision.
 *
 * Beyond forwarding pre/post hooks to the registered output schemes, this action resets
 * the global time of secondaries emitted by an initial radioactive decay (so that decay
 * chains start at t=0) and issues a warning once the global time grows large enough that
 * @c double precision degrades the timing resolution below 1 us.
 */
class RMGTrackingAction : public G4UserTrackingAction {

  public:

    RMGTrackingAction(RMGRunAction*);
    ~RMGTrackingAction() = default;

    RMGTrackingAction(RMGTrackingAction const&) = delete;
    RMGTrackingAction& operator=(RMGTrackingAction const&) = delete;
    RMGTrackingAction(RMGTrackingAction&&) = delete;
    RMGTrackingAction& operator=(RMGTrackingAction&&) = delete;

    /** @brief Forward the track to all output schemes for pre-tracking bookkeeping. */
    void PreUserTrackingAction(const G4Track*) override;
    /**
     * @brief Run post-tracking output bookkeeping, reset initial-decay times and emit a
     * one-shot warning when global time grows large enough to spoil sub-us precision.
     */
    void PostUserTrackingAction(const G4Track*) override;
    /** @brief Access the underlying Geant4 tracking manager (for output-scheme integration). */
    G4TrackingManager* GetTrackingManager() { return G4UserTrackingAction::fpTrackingManager; };

  private:

    RMGRunAction* fRunAction = nullptr;
    bool fResetInitialDecayTime = true;
    bool fHadLongTimeWarning = false;
    double fMaxRepresentableGlobalTime = -1;

    bool ResetInitialDecayTime(const G4Track*);

    void SetLongGlobalTimeUncertaintyWarning(double);

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

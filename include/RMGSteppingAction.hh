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

#ifndef _RMG_STEPPING_ACTION_HH_
#define _RMG_STEPPING_ACTION_HH_

#include <memory>

#include "G4GenericMessenger.hh"
#include "G4UserSteppingAction.hh"

class G4Step;
class RMGRunAction;
/**
 * @brief Stepping action driving output schemes and optional kill heuristics.
 *
 * In addition to forwarding each step to the registered output schemes, this action can
 * be configured to drop secondary tracks and to kill long-lived daughter nuclei (see
 * @ref SetDaughterKillLifetime) — useful e.g. to truncate decay chains that would otherwise
 * extend over long timescales.
 */
class RMGSteppingAction : public G4UserSteppingAction {

  public:

    RMGSteppingAction(RMGRunAction*);
    ~RMGSteppingAction() = default;

    RMGSteppingAction(RMGSteppingAction const&) = delete;
    RMGSteppingAction& operator=(RMGSteppingAction const&) = delete;
    RMGSteppingAction(RMGSteppingAction&&) = delete;
    RMGSteppingAction& operator=(RMGSteppingAction&&) = delete;

    /**
     * @brief Forward the step to all output schemes and apply tracking-control of long-lived isotopes.
     */
    void UserSteppingAction(const G4Step*) override;

    /**
     * @brief Kill daughter nuclei whose PDG lifetime exceeds @p max_lifetime.
     * @details Applies to ground-state nuclei produced as secondaries; the cut is on the
     * tabulated half-life of the species, not on the sampled decay time. Set to a
     * non-positive value to disable.
     * @param max_lifetime Maximum allowed lifetime, in Geant4 units of time.
     */
    void SetDaughterKillLifetime(double max_lifetime);

  private:

    RMGRunAction* fRunAction;

    bool fSkipTracking = false;
    bool fKillSecondaries = false;
    double fDaughterKillLifetime = -1;

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

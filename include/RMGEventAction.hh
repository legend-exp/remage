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

#ifndef _RMG_EVENT_ACTION_HH_
#define _RMG_EVENT_ACTION_HH_

#include <memory>

#include "G4Event.hh"
#include "G4UserEventAction.hh"

class RMGRunAction;
/**
 * @brief Per-event user action driving the output schemes.
 *
 * Dispatches begin/end-of-event hooks to all output schemes registered on the owning
 * @ref RMGRunAction, takes care of clearing per-event buffers and decides whether to write
 * the event ntuple based on the schemes' filtering criteria.
 */
class RMGEventAction : public G4UserEventAction {

  public:

    RMGEventAction(RMGRunAction*);
    ~RMGEventAction() = default;

    RMGEventAction(RMGEventAction const&) = delete;
    RMGEventAction& operator=(RMGEventAction const&) = delete;
    RMGEventAction(RMGEventAction&&) = delete;
    RMGEventAction& operator=(RMGEventAction&&) = delete;

    /** @brief Clear per-event output buffers and notify schemes that a new event has started. */
    void BeginOfEventAction(const G4Event*) override;
    /** @brief Run filtering schemes and, if the event is kept, fill the output ntuples. */
    void EndOfEventAction(const G4Event*) override;

  private:

    RMGRunAction* fRunAction = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

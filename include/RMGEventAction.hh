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

#ifndef _RMG_EVENT_ACTION_HH_
#define _RMG_EVENT_ACTION_HH_

#include <memory>

#include "G4Event.hh"
#include "G4UserEventAction.hh"

class RMGRunAction;
class G4GenericMessenger;
class RMGVOutputManager;
class RMGEventAction : public G4UserEventAction {

  public:

    RMGEventAction(RMGRunAction*);
    inline ~RMGEventAction() = default;

    RMGEventAction(RMGEventAction const&) = delete;
    RMGEventAction& operator=(RMGEventAction const&) = delete;
    RMGEventAction(RMGEventAction&&) = delete;
    RMGEventAction& operator=(RMGEventAction&&) = delete;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

  private:

    RMGRunAction* fRunAction = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

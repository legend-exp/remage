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

#ifndef _RMG_MANAGEMENT_TRACKING_ACTION_HH_
#define _RMG_MANAGEMENT_TRACKING_ACTION_HH_

#include "G4UserTrackingAction.hh"

class RMGEventAction;
class RMGTrackingAction : public G4UserTrackingAction {

  public:

    RMGTrackingAction(RMGEventAction*);
    ~RMGTrackingAction() = default;

    RMGTrackingAction(RMGTrackingAction const&) = delete;
    RMGTrackingAction& operator=(RMGTrackingAction const&) = delete;
    RMGTrackingAction(RMGTrackingAction&&) = delete;
    RMGTrackingAction& operator=(RMGTrackingAction&&) = delete;

    virtual void PreUserTrackingAction(const G4Track*) override;
    virtual void PostUserTrackingAction(const G4Track*) override;
    inline G4TrackingManager* GetTrackingManager() {
      return G4UserTrackingAction::fpTrackingManager;
    };

  private:

    RMGEventAction* fEventAction = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

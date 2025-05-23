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

#ifndef _RMG_USER_ACTION_HH_
#define _RMG_USER_ACTION_HH_

#include "G4VUserActionInitialization.hh"

class RMGUserAction : public G4VUserActionInitialization {

  public:

    RMGUserAction() = default;
    ~RMGUserAction() override = default;

    RMGUserAction(RMGUserAction const&) = delete;
    RMGUserAction& operator=(RMGUserAction const&) = delete;
    RMGUserAction(RMGUserAction&&) = delete;
    RMGUserAction& operator=(RMGUserAction&&) = delete;

    void Build() const override;
    void BuildForMaster() const override;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

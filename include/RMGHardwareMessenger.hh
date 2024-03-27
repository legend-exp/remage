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

#ifndef _RMG_HARDWARE_MESSENGER_HH_
#define _RMG_HARDWARE_MESSENGER_HH_

#include "G4UImessenger.hh"

class RMGHardware;
class RMGHardwareMessenger : public G4UImessenger {

  public:

    RMGHardwareMessenger(RMGHardware* hw);
    ~RMGHardwareMessenger();

    void SetNewValue(G4UIcommand* command, G4String newValues);

  private:

    RMGHardware* fHardware;
    G4UIcommand* fRegisterCmd;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

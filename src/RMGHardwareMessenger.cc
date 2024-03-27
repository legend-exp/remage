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

#include "RMGHardwareMessenger.hh"

#include "RMGHardware.hh"

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include "fmt/core.h"
#include "magic_enum/magic_enum.hpp"

RMGHardwareMessenger::RMGHardwareMessenger(RMGHardware* hw) : fHardware(hw) {
  auto detector_types = magic_enum::enum_names<RMGHardware::DetectorType>();
  auto options = fmt::format("{}", fmt::join(detector_types, "|"));

  fRegisterCmd = new G4UIcommand("/RMG/Geometry/RegisterDetector", this);
  fRegisterCmd->SetGuidance("register a sensitive detector");
  fRegisterCmd->SetGuidance("[usage] /RMG/Geometry/RegisterDetector T PV ID [C]");
  fRegisterCmd->SetGuidance(fmt::format("        T:(str) {}", options).c_str());
  fRegisterCmd->SetGuidance("        PV:(s) physvol");
  fRegisterCmd->SetGuidance("        ID:(int) unique detector id");
  fRegisterCmd->SetGuidance("        C:(int) copy nr (default 0)");

  fRegisterCmd->SetParameter(new G4UIparameter("type", 's', false));
  fRegisterCmd->SetParameter(new G4UIparameter("pv_name", 's', false));
  fRegisterCmd->SetParameter(new G4UIparameter("uid", 'i', false));
  fRegisterCmd->SetParameter(new G4UIparameter("copy_nr", 'i', true));

  fRegisterCmd->AvailableForStates(G4State_PreInit);
}

RMGHardwareMessenger::~RMGHardwareMessenger() { delete fRegisterCmd; }

void RMGHardwareMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
  if (command == fRegisterCmd) fHardware->RegisterDetectorCmd(newValues);
}

// vim: tabstop=2 shiftwidth=2 expandtab

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

#include "RMGHardwareMessenger.hh"

#include "G4Tokenizer.hh"
#include "G4UIcommand.hh"

#include "RMGHardware.hh"
#include "RMGTools.hh"

RMGHardwareMessenger::RMGHardwareMessenger(RMGHardware* hw) : fHardware(hw) {

  this->DefineRegisterDetector();
  this->DefineStepLimits();
}

RMGHardwareMessenger::~RMGHardwareMessenger() { delete fRegisterCmd; }

void RMGHardwareMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

  if (command == fRegisterCmd) RegisterDetectorCmd(newValues);
  else if (command == fStepLimitsCmd) StepLimitsCmd(newValues);
}

void RMGHardwareMessenger::DefineRegisterDetector() {

  fRegisterCmd = new G4UIcommand("/RMG/Geometry/RegisterDetector", this);
  fRegisterCmd->SetGuidance("register a sensitive detector");

  auto p_type = new G4UIparameter("type", 's', false);
  p_type->SetParameterCandidates(RMGTools::GetCandidates<RMGDetectorType>().c_str());
  p_type->SetGuidance("Detector type");
  fRegisterCmd->SetParameter(p_type);

  auto p_pv = new G4UIparameter("pv_name", 's', false);
  p_pv->SetGuidance("Detector physical volume");
  fRegisterCmd->SetParameter(p_pv);

  auto p_uid = new G4UIparameter("uid", 'i', false);
  p_uid->SetGuidance("unique detector id");
  fRegisterCmd->SetParameter(p_uid);

  auto p_copy = new G4UIparameter("copy_nr", 'i', true);
  p_copy->SetGuidance("copy nr (default 0)");
  p_copy->SetDefaultValue("0");
  fRegisterCmd->SetParameter(p_copy);

  auto p_reuse = new G4UIparameter("allow_id_reuse", 'b', true);
  p_reuse->SetGuidance(
      "append this volume to a previously allocated unique detector id, instead of erroring out."
  );
  p_reuse->SetDefaultValue("false");
  fRegisterCmd->SetParameter(p_reuse);

  fRegisterCmd->AvailableForStates(G4State_PreInit);
}

void RMGHardwareMessenger::DefineStepLimits() {

  fStepLimitsCmd = new G4UIcmdWithADoubleAndUnit("/RMG/Geometry/SetMaxStepSize", this);
  fStepLimitsCmd->SetGuidance("Sets maximum step size for a certain detector");

  fStepLimitsCmd->SetParameterName("step_size", false);
  fStepLimitsCmd->SetDefaultValue(1);
  fStepLimitsCmd->SetUnitCategory("Length");

  auto p_pv = new G4UIparameter("pv_name", 's', false);
  p_pv->SetGuidance("Detector physical volume");
  fStepLimitsCmd->SetParameter(p_pv);

  fStepLimitsCmd->AvailableForStates(G4State_PreInit);
}

void RMGHardwareMessenger::RegisterDetectorCmd(const std::string& parameters) {
  G4Tokenizer next(parameters);

  auto type_str = next();
  auto type = RMGTools::ToEnum<RMGDetectorType>(std::string(type_str), "detector type");
  auto pv_name = next();
  const int uid = std::stoi(next());
  int copy_nr = 0;
  auto copy_nr_str = next();
  if (!copy_nr_str.empty()) copy_nr = std::stoi(copy_nr_str);
  bool allow_reuse = false;
  auto allow_reuse_str = next();
  if (!allow_reuse_str.empty()) allow_reuse = G4UIcommand::ConvertToBool(allow_reuse_str);

  fHardware->RegisterDetector(type, pv_name, uid, copy_nr, allow_reuse);
}

void RMGHardwareMessenger::StepLimitsCmd(const std::string& parameters) {
  G4Tokenizer next(parameters);

  auto x = next();
  auto y = next();
  auto num = G4UIcmdWithADoubleAndUnit::GetNewDoubleValue((x + " " + y).c_str());
  auto pv_name = next();

  fHardware->SetMaxStepLimit(num, pv_name);
}

// vim: tabstop=2 shiftwidth=2 expandtab

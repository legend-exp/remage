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

#include "RMGOutputManager.hh"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "G4AnalysisManager.hh"
#include "Randomize.hh"

#include "RMGConfig.hh"
#include "RMGExceptionHandler.hh"
#include "RMGIpc.hh"
#include "RMGManager.hh"
#include "RMGUserAction.hh"
#include "RMGUserInit.hh"

RMGOutputManager* RMGOutputManager::fRMGOutputManager = nullptr;

G4ThreadLocal std::map<int, std::pair<int, std::string>> RMGOutputManager::fNtupleIDs = {};
G4ThreadLocal std::map<std::string, int> RMGOutputManager::fNtupleAuxIDs = {};

RMGOutputManager::RMGOutputManager() {

  if (fRMGOutputManager) RMGLog::Out(RMGLog::fatal, "RMGOutputManager must be singleton!");
  fRMGOutputManager = this;

  this->DefineCommands();
}

int RMGOutputManager::RegisterNtuple(int det_uid, int ntuple_id, std::string table_name) {
  auto res = fNtupleIDs.emplace(det_uid, std::make_pair(ntuple_id, table_name));
  if (!res.second)
    RMGLog::OutFormatDev(RMGLog::fatal, "Ntuple for detector with UID {} is already registered", det_uid);
  return this->GetNtupleID(det_uid);
}

int RMGOutputManager::CreateAndRegisterNtuple(
    int det_uid,
    std::string table_name,
    std::string oscheme,
    G4AnalysisManager* ana_man
) {
  auto ntuple_id = ana_man->CreateNtuple(table_name, oscheme);
  ntuple_id = this->RegisterNtuple(det_uid, ntuple_id, table_name);
  RMGIpc::SendIpcNonBlocking(
      RMGIpc::CreateMessage("output_ntuple", std::string(oscheme).append("\x1e").append(table_name))
  );
  return ntuple_id;
}

int RMGOutputManager::CreateAndRegisterAuxNtuple(
    std::string table_name,
    std::string oscheme,
    G4AnalysisManager* ana_man
) {
  auto ntuple_id = ana_man->CreateNtuple(table_name, oscheme);
  auto res = fNtupleAuxIDs.emplace(table_name, ntuple_id);
  if (!res.second)
    RMGLog::OutFormatDev(RMGLog::fatal, "Ntuple for table with UID {} is already registered", table_name);
  RMGIpc::SendIpcNonBlocking(
      RMGIpc::CreateMessage("output_ntuple_aux", std::string(oscheme).append("\x1e").append(table_name))
  );
  return this->GetAuxNtupleID(table_name);
}

void RMGOutputManager::ActivateOptionalOutputScheme(std::string name) {
  RMGManager::Instance()->ActivateOptionalOutputScheme(name);
}

void RMGOutputManager::DefineCommands() {

  fOutputMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Output/",
      "Commands for controlling the simulation output"
  );

  fOutputMessenger->DeclareMethod("FileName", &RMGOutputManager::SetOutputFileName)
      .SetGuidance("Set output file name for object persistency")
      .SetParameterName("filename", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fOutputMessenger->DeclareProperty("NtuplePerDetector", fOutputNtuplePerDetector)
      .SetGuidance(
          "Create a ntuple for each sensitive detector to store hits. Otherwise, store "
          "all hits of one detector type in one ntuple."
      )
      .SetParameterName("nt_per_det", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fOutputMessenger->DeclareProperty("NtupleUseVolumeName", fOutputNtupleUseVolumeName)
      .SetGuidance("Use the sensitive volume name to name output ntuples.")
      .SetGuidance("note: this only works if `NtuplePerDetector` is set to true.")
      .SetParameterName("nt_vol_name", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fOutputMessenger
      ->DeclareMethod("ActivateOutputScheme", &RMGOutputManager::ActivateOptionalOutputScheme)
      .SetGuidance("Activates the output scheme that had been registered under the given name.")
      .SetParameterName("oscheme", false)
      .SetStates(G4State_PreInit);

  fOutputMessenger->DeclareMethod("NtupleDirectory", &RMGOutputManager::SetOutputNtupleDirectory)
      .SetGuidance("Change the default output directory/group for ntuples in output files.")
      .SetGuidance("note: This setting is not respected by all output formats.")
      .SetParameterName("nt_directory", false)
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

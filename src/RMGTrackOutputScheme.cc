// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#include "RMGTrackOutputScheme.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"

#include "RMGLog.hh"
#include "RMGOutputManager.hh"

namespace u = CLHEP;

RMGTrackOutputScheme::RMGTrackOutputScheme() { this->DefineCommands(); }

// invoked in RMGRunAction::SetupAnalysisManager()
void RMGTrackOutputScheme::AssignOutputNames(G4AnalysisManager* ana_man) {
  auto vid = RMGOutputManager::Instance()
                 ->CreateAndRegisterAuxNtuple("tracks", "RMGTrackOutputScheme", ana_man);

  ana_man->CreateNtupleIColumn(vid, "evtid");
  ana_man->CreateNtupleIColumn(vid, "trackid");
  ana_man->CreateNtupleIColumn(vid, "parent_trackid");
  ana_man->CreateNtupleIColumn(vid, "procid");
  ana_man->CreateNtupleIColumn(vid, "particle");
  ana_man->CreateNtupleDColumn(vid, "time_in_ns");
  CreateNtupleFOrDColumn(ana_man, vid, "xloc_in_m", fStoreSinglePrecisionPosition);
  CreateNtupleFOrDColumn(ana_man, vid, "yloc_in_m", fStoreSinglePrecisionPosition);
  CreateNtupleFOrDColumn(ana_man, vid, "zloc_in_m", fStoreSinglePrecisionPosition);
  CreateNtupleFOrDColumn(ana_man, vid, "px_in_MeV", fStoreSinglePrecisionEnergy);
  CreateNtupleFOrDColumn(ana_man, vid, "py_in_MeV", fStoreSinglePrecisionEnergy);
  CreateNtupleFOrDColumn(ana_man, vid, "pz_in_MeV", fStoreSinglePrecisionEnergy);
  CreateNtupleFOrDColumn(ana_man, vid, "ekin_in_MeV", fStoreSinglePrecisionEnergy);

  ana_man->FinishNtuple(vid);

  auto pid = RMGOutputManager::Instance()
                 ->CreateAndRegisterAuxNtuple("processes", "RMGTrackOutputScheme", ana_man);
  ana_man->CreateNtupleIColumn(pid, "procid");
  ana_man->CreateNtupleSColumn(pid, "name");
  ana_man->FinishNtuple(pid);
}

void RMGTrackOutputScheme::TrackingActionPre(const G4Track* track) {
  auto rmg_man = RMGOutputManager::Instance();
  if (!rmg_man->IsPersistencyEnabled()) return;

  const auto ana_man = G4AnalysisManager::Instance();

  // do never write tracks of optical photons (there will be many).
  if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) { return; }

  auto pos = track->GetPosition();
  auto primary = track->GetDynamicParticle();
  auto proc = track->GetCreatorProcess();

  std::string proc_name;
  if (proc) proc_name = proc->GetProcessName();

  auto write = true;
  write &= (fFilterProcess.empty() || fFilterProcess.find(proc_name) != fFilterProcess.end());
  write &=
      (fFilterParticle.empty() ||
       fFilterParticle.find(primary->GetPDGcode()) != fFilterParticle.end());
  write &= (fFilterEnergy == -1 || track->GetKineticEnergy() >= fFilterEnergy);
  if (!write) return;

  int proc_id = -1;
  if (proc) {
    if (fProcessMap.find(proc_name) == fProcessMap.end()) {
      // The following lines are a FNV-1a hash function (based on the CC0 licensed algorithm)
      // see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
      // and http://www.isthe.com/chongo/tech/comp/fnv/index.html for details.
      uint32_t proc_name_hash = 0x811c9dc5;
      for (char& b : proc_name) {
        proc_name_hash ^= (uint32_t)b;
        proc_name_hash *= 0x01000193;
      }
      // xor-fold down to 16 bit.
      proc_name_hash = (proc_name_hash >> 16) ^ (proc_name_hash & (uint32_t)0xffff);
      fProcessMap.emplace(proc_name, proc_name_hash);
    }
    proc_id = static_cast<int>(fProcessMap[proc_name]);
  }

  auto ntupleid = rmg_man->GetAuxNtupleID("tracks");
  int col_id = 0;
  ana_man->FillNtupleIColumn(
      ntupleid,
      col_id++,
      G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
  );
  ana_man->FillNtupleIColumn(ntupleid, col_id++, track->GetTrackID());
  ana_man->FillNtupleIColumn(ntupleid, col_id++, track->GetParentID());
  ana_man->FillNtupleIColumn(ntupleid, col_id++, proc_id);
  ana_man->FillNtupleIColumn(ntupleid, col_id++, primary->GetPDGcode());
  ana_man->FillNtupleDColumn(ntupleid, col_id++, track->GetGlobalTime() / u::ns);
  FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, pos.getX() / u::m, fStoreSinglePrecisionPosition);
  FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, pos.getY() / u::m, fStoreSinglePrecisionPosition);
  FillNtupleFOrDColumn(ana_man, ntupleid, col_id++, pos.getZ() / u::m, fStoreSinglePrecisionPosition);
  FillNtupleFOrDColumn(
      ana_man,
      ntupleid,
      col_id++,
      primary->GetMomentum().getX() / u::MeV,
      fStoreSinglePrecisionEnergy
  );
  FillNtupleFOrDColumn(
      ana_man,
      ntupleid,
      col_id++,
      primary->GetMomentum().getY() / u::MeV,
      fStoreSinglePrecisionEnergy
  );
  FillNtupleFOrDColumn(
      ana_man,
      ntupleid,
      col_id++,
      primary->GetMomentum().getZ() / u::MeV,
      fStoreSinglePrecisionEnergy
  );
  FillNtupleFOrDColumn(
      ana_man,
      ntupleid,
      col_id++,
      track->GetKineticEnergy() / u::MeV,
      fStoreSinglePrecisionEnergy
  );

  ana_man->AddNtupleRow(ntupleid);
}

void RMGTrackOutputScheme::EndOfRunAction(const G4Run*) {
  auto rmg_man = RMGOutputManager::Instance();
  if (!rmg_man->IsPersistencyEnabled()) return;

  const auto ana_man = G4AnalysisManager::Instance();
  auto ntupleid = rmg_man->GetAuxNtupleID("processes");

  std::set<uint32_t> proc_ids; // to check for duplicates.

  for (auto& [proc_name, proc_id] : fProcessMap) {
    ana_man->FillNtupleIColumn(ntupleid, 0, static_cast<int>(proc_id));
    ana_man->FillNtupleSColumn(ntupleid, 1, proc_name);
    ana_man->AddNtupleRow(ntupleid);

    if (!proc_ids.insert(proc_id).second) {
      RMGLog::OutDev(RMGLog::error, "duplicate process name hash ", proc_id);
    }
  }
}


void RMGTrackOutputScheme::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Output/Track/",
      "Commands for controlling output of track vertices."
  );

  fMessenger->DeclareMethod("AddProcessFilter", &RMGTrackOutputScheme::AddProcessFilter)
      .SetGuidance("Only include tracks created by this process.")
      .SetParameterName("process", false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("AddParticleFilter", &RMGTrackOutputScheme::AddParticleFilter)
      .SetGuidance("Only include tracks with this particle.")
      .SetParameterName("pdgid", false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareMethod("EnergyFilter", &RMGTrackOutputScheme::SetEnergyFilter)
      .SetGuidance("Only include tracks with kinetic energy above this threshold.")
      .SetParameterName("energy", false)
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionPosition", fStoreSinglePrecisionPosition)
      .SetGuidance("Use float32 (instead of float64) for position output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fMessenger->DeclareProperty("StoreSinglePrecisionEnergy", fStoreSinglePrecisionEnergy)
      .SetGuidance("Use float32 (instead of float64) for energy output.")
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab

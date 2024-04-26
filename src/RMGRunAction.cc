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

#include "RMGRunAction.hh"

#include <cstdlib>
#include <ctime>
#include <limits>

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

#include "RMGEventAction.hh"
#include "RMGGermaniumOutputScheme.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGMasterGenerator.hh"
#include "RMGOpticalOutputScheme.hh"
#include "RMGRun.hh"
#include "RMGVGenerator.hh"

#include "fmt/chrono.h"

G4Run* RMGRunAction::GenerateRun() {
  fRMGRun = new RMGRun();
  return fRMGRun;
}

RMGRunAction::RMGRunAction(bool persistency) : fIsPersistencyEnabled(persistency) {}

RMGRunAction::RMGRunAction(RMGMasterGenerator* gene, bool persistency)
    : fIsPersistencyEnabled(persistency), fRMGMasterGenerator(gene) {}

// called at the begin of the run action.
void RMGRunAction::SetupAnalysisManager() {
  if (fIsAnaManInitialized) return;
  fIsAnaManInitialized = true;

  auto rmg_man = RMGManager::Instance();
  if (rmg_man->GetDetectorConstruction()->GetActiveDetectorList().empty()) {
    rmg_man->EnablePersistency(false);
    fIsPersistencyEnabled = false;
  }

  RMGLog::Out(RMGLog::debug, "Setting up analysis manager");

  auto ana_man = G4AnalysisManager::Instance();

  // otherwise the ntuples get placed in /default_ntuples (at least with HDF5 output)
  ana_man->SetNtupleDirectoryName("hit");

  if (RMGLog::GetLogLevel() <= RMGLog::debug) ana_man->SetVerboseLevel(10);
  else ana_man->SetVerboseLevel(0);

  ana_man->SetNtupleMerging(!RMGManager::Instance()->IsExecSequential());

  // do it only for activated detectors (have to ask to the manager)
  auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  for (const auto& d_type : det_cons->GetActiveDetectorList()) {

    fOutputDataFields[d_type] = det_cons->GetActiveOutputScheme(d_type);
    if (!fOutputDataFields[d_type]) {
      RMGLog::OutDev(RMGLog::fatal, "No output scheme sensitive detector type '",
          magic_enum::enum_name(d_type), "' implemented (implement me)");
    }

    fOutputDataFields[d_type]->AssignOutputNames(ana_man);
  }
}

void RMGRunAction::BeginOfRunAction(const G4Run*) {

  RMGLog::OutDev(RMGLog::debug, "Start of run action");

  auto manager = RMGManager::Instance();

  if (fIsPersistencyEnabled) this->SetupAnalysisManager();

  // Check again, SetupAnalysisManager might have modified fIsPersistencyEnabled.
  if (fIsPersistencyEnabled) {
    auto ana_man = G4AnalysisManager::Instance();
    // TODO: realpath
    if (this->IsMaster())
      RMGLog::Out(RMGLog::summary, "Opening output file: ", manager->GetOutputFileName());
    ana_man->OpenFile(manager->GetOutputFileName());
  } else if (this->IsMaster()) {
    // Warn user if persistency is disabled if there are detectors defined.
    auto level = manager->GetDetectorConstruction()->GetActiveDetectorList().empty()
                     ? RMGLog::summary
                     : RMGLog::warning;
    RMGLog::Out(level, "Object persistency disabled");
  }

  if (fRMGMasterGenerator) {
    if (fRMGMasterGenerator->GetVertexGenerator()) {
      fRMGMasterGenerator->GetVertexGenerator()->BeginOfRunAction(fRMGRun);
    }
    if (fRMGMasterGenerator->GetGenerator()) {
      fRMGMasterGenerator->GetGenerator()->BeginOfRunAction(fRMGRun);
    }
  }

  // save start time for future
  fRMGRun->SetStartTime(std::chrono::system_clock::now());

  if (this->IsMaster()) {
    auto tt = fmt::localtime(fRMGRun->GetStartTime());

    RMGLog::OutFormat(RMGLog::summary,
        "Starting run nr. {:d}. Current local time is {:%d-%m-%Y %H:%M:%S}", fRMGRun->GetRunID(), tt);
    RMGLog::OutFormat(RMGLog::summary, "Number of events to be processed: {:d}",
        fRMGRun->GetNumberOfEventToBeProcessed());
  }

  auto g4manager = G4RunManager::GetRunManager();
  auto tot_events = g4manager->GetNumberOfEventsToBeProcessed();

  fCurrentPrintModulo = manager->GetPrintModulo();
  if (fCurrentPrintModulo <= 0 and tot_events >= 100) fCurrentPrintModulo = tot_events / 10;
  else if (tot_events < 100) fCurrentPrintModulo = 100;
}

void RMGRunAction::EndOfRunAction(const G4Run*) {

  RMGLog::OutDev(RMGLog::debug, "End of run action");

  // report some stats
  if (this->IsMaster()) {
    auto time_now = std::chrono::system_clock::now();

    int n_ev = fRMGRun->GetNumberOfEvent();
    int n_ev_requested = fRMGRun->GetNumberOfEventToBeProcessed();

    RMGLog::OutFormat(RMGLog::summary,
        "Run nr. {:d} completed. {:d} events simulated. Current local time is {:%d-%m-%Y %H:%M:%S}",
        fRMGRun->GetRunID(), n_ev, fmt::localtime(time_now));
    if (n_ev != n_ev_requested) {
      RMGLog::OutFormat(RMGLog::warning,
          "Run nr. {:d} only simulated {:d} events, out of {:d} events requested!",
          fRMGRun->GetRunID(), n_ev, n_ev_requested);
    }

    auto start_time = fRMGRun->GetStartTime();
    auto tot_elapsed_s =
        std::chrono::duration_cast<std::chrono::seconds>(time_now - start_time).count();
    long partial = 0;
    long elapsed_d = (tot_elapsed_s - partial) / 86400;
    partial += elapsed_d * 86400;
    long elapsed_h = (tot_elapsed_s - partial) / 3600;
    partial += elapsed_h * 3600;
    long elapsed_m = (tot_elapsed_s - partial) / 60;
    partial += elapsed_m * 60;
    long elapsed_s = tot_elapsed_s - partial;

    RMGLog::OutFormat(RMGLog::summary,
        "Stats: run time was {:d} days, {:d} hours, {:d} minutes and {:d} seconds", elapsed_d,
        elapsed_h, elapsed_m, elapsed_s);

    auto total_sec_hres =
        (double)std::chrono::duration<double>(time_now - fRMGRun->GetStartTime()).count();

    RMGLog::OutFormat(RMGLog::summary,
        "Stats: average event processing time was {:.5g} seconds/event = {:.5g} events/second",
        total_sec_hres / n_ev, n_ev / total_sec_hres);

    if (n_ev < 100)
      RMGLog::Out(RMGLog::summary, "Stats: Event processing time might be inaccurate");
  }

  if (fRMGMasterGenerator) {
    if (fRMGMasterGenerator->GetVertexGenerator()) {
      fRMGMasterGenerator->GetVertexGenerator()->EndOfRunAction(fRMGRun);
    }
    if (fRMGMasterGenerator->GetGenerator()) {
      fRMGMasterGenerator->GetGenerator()->EndOfRunAction(fRMGRun);
    }
  }

  if (fIsPersistencyEnabled) {
    G4AnalysisManager::Instance()->Write();
    G4AnalysisManager::Instance()->CloseFile();
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

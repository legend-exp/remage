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
#include <random>

#include "G4AnalysisManager.hh"
#include "G4AnalysisUtilities.hh"
#include "G4AutoLock.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

#include "RMGConfig.hh"
#if RMG_HAS_HDF5
#include "RMGConvertLH5.hh"
#endif
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
  auto det_cons = rmg_man->GetDetectorConstruction();
  if (det_cons->GetAllActiveOutputSchemes().empty()) {
    rmg_man->EnablePersistency(false);
    fIsPersistencyEnabled = false;
  }

  RMGLog::Out(RMGLog::debug, "Setting up analysis manager");

  auto ana_man = G4AnalysisManager::Instance();

  // otherwise the ntuples get placed in /default_ntuples (at least with HDF5 output)
  ana_man->SetNtupleDirectoryName(rmg_man->GetOutputNtupleDirectory());

  if (RMGLog::GetLogLevel() <= RMGLog::debug) ana_man->SetVerboseLevel(10);
  else ana_man->SetVerboseLevel(0);

  // do it only for activated detectors (have to ask to the manager)
  for (const auto& oscheme : det_cons->GetAllActiveOutputSchemes()) {

    fOutputDataFields.emplace_back(oscheme);

    oscheme->SetNtuplePerDetector(rmg_man->GetOutputNtuplePerDetector());
    oscheme->SetNtupleUseVolumeName(rmg_man->GetOutputNtupleUseVolumeName());
    oscheme->AssignOutputNames(ana_man);
  }
}

void RMGRunAction::BeginOfRunAction(const G4Run*) {

  RMGLog::OutDev(RMGLog::debug, "Start of run action");

  auto manager = RMGManager::Instance();

  if (fIsPersistencyEnabled) this->SetupAnalysisManager();

  // Check again, SetupAnalysisManager might have modified fIsPersistencyEnabled.
  if (fIsPersistencyEnabled) {
    auto ana_man = G4AnalysisManager::Instance();
    fCurrentOutputFile = BuildOutputFile();
    auto fn = fCurrentOutputFile.first.string();

    // ntuple merging is only supported for some file types. Unfortunately, the function to
    // check for this capability is private, so we have to replicate this here. Also it can only be
    // called after opening the file, when setting the flag does not work any more :-(
    auto file_type = fCurrentOutputFile.first.extension();
    if (file_type != ".csv" && file_type != ".CSV" && file_type != ".xml" && file_type != ".XML" &&
        file_type != ".hdf5" && file_type != ".HDF5") {
      ana_man->SetNtupleMerging(!manager->IsExecSequential());
    }

    if (this->IsMaster()) {
      std::string orig_fn;
      if (fCurrentOutputFile.first != fCurrentOutputFile.second)
        orig_fn = " (for " + fCurrentOutputFile.second.string() + ")";
      RMGLog::Out(RMGLog::summary, "Opening output file: ", fn, orig_fn);
    }
    if (fCurrentOutputFile.first != fCurrentOutputFile.second && std::filesystem::exists(fn)) {
      RMGLog::Out(RMGLog::fatal, "Temporary file ", fn, " already exists?");
    }
    auto success = ana_man->OpenFile(fn);

    // If opening failed, disable persistency.
    if (!success) {
      if (this->IsMaster()) RMGLog::Out(RMGLog::error, "Failed opening output file ", fn);
      manager->EnablePersistency(false);
      fIsPersistencyEnabled = false;
    }
  }

  if (!fIsPersistencyEnabled && this->IsMaster()) {
    // Warn user if persistency is disabled if there are detectors defined.
    auto level = manager->GetDetectorConstruction()->GetAllActiveOutputSchemes().empty()
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

  auto oschemes = GetAllOutputDataFields();
  for (const auto& oscheme : oschemes) { oscheme->EndOfRunAction(fRMGRun); }

  if (fIsPersistencyEnabled) {
    G4AnalysisManager::Instance()->Write();
    G4AnalysisManager::Instance()->CloseFile();

    PostprocessOutputFile();
  }
}

// Geant4 cannot handle LH5 files by default, and there is also no way to teach it another file
// extension. So if the user specifies a LH5 file as output, we have to create a temporary file
// with a hdf5 extensions. Later, we will rename it.

std::pair<fs::path, fs::path> RMGRunAction::BuildOutputFile() const {
  auto manager = RMGManager::Instance();

  // TODO: realpath
  auto path = fs::path(manager->GetOutputFileName());
  auto path_for_overwrite =
      fs::path(G4Analysis::GetTnFileName(path.string(), path.extension().string()));
  if (fs::exists(path_for_overwrite) && !manager->GetOutputOverwriteFiles()) {
    RMGLog::Out(RMGLog::error, "Output file ", path_for_overwrite.string(), " does already exists.");
    // need to abort here (and not use ::fatal), as Geant4 is not prepared to take exceptions at
    // this stage. also, there is no clean way to abort an run before the run started.
    std::abort();
  }

  auto ext = path.extension();
  if (ext == ".lh5" || ext == ".LH5") {
#if !RMG_HAS_HDF5
    RMGLog::Out(RMGLog::fatal, "HDF5 and LH5 support is not available!");
#endif
    std::uniform_int_distribution<int> dist(10000, 99999);
    std::random_device rd;

    std::string new_fn =
        ".rmg-tmp-" + std::to_string(dist(rd)) + "." + path.stem().string() + ".hdf5";
    auto new_path = path.parent_path() / new_fn;
    return {new_path, path};
  }
  return {path, path};
}

namespace {
  G4Mutex RMGConvertLH5Mutex = G4MUTEX_INITIALIZER;
}

void RMGRunAction::PostprocessOutputFile() const {
  if (fCurrentOutputFile.first == fCurrentOutputFile.second) return;

  // HDF5 C++ might not be thread-safe?
  G4AutoLock l(&RMGConvertLH5Mutex);

  auto worker_tmp = fs::path(G4Analysis::GetTnFileName(fCurrentOutputFile.first.string(), "hdf5"));
  auto worker_lh5 = fs::path(G4Analysis::GetTnFileName(fCurrentOutputFile.second.string(), "lh5"));

  if (!fs::exists(worker_tmp)) {
    if (!this->IsMaster() || RMGManager::Instance()->IsExecSequential()) {
      RMGLog::Out(RMGLog::error, "Temporary output file ", worker_tmp.string(),
          " not found for conversion.");
    }
    return;
  }

#if RMG_HAS_HDF5
  auto nt_dir = RMGManager::Instance()->GetOutputNtupleDirectory();
  // note: do not do a dry-run here, as it takes a lot of memory.
  if (!RMGConvertLH5::ConvertToLH5(worker_tmp.string(), nt_dir, false)) {
    RMGLog::Out(RMGLog::error, "Conversion of output file ", worker_tmp.string(),
        " to LH5 failed. Data is potentially corrupted.");
    return;
  }
#else
  RMGLog::OutDev(RMGLog::fatal, "HDF5 and LH5 support is not available!");
#endif

  try {
    fs::rename(worker_tmp, worker_lh5);
    RMGLog::Out(RMGLog::summary, "Moved output file ", worker_tmp.string(), " to ",
        worker_lh5.string());
  } catch (const fs::filesystem_error& e) {
    RMGLog::Out(RMGLog::error, "Moving output file ", worker_tmp.string(), " to ",
        worker_lh5.string(), " failed: ", e.what());
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

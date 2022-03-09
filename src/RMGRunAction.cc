#include "RMGRunAction.hh"

#include <limits>
#include <ctime>
#include <cstdlib>

#include "G4Run.hh"
#include "G4RunManager.hh"

#include "RMGRun.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGMasterGenerator.hh"
#include "RMGVGenerator.hh"
#include "RMGEventAction.hh"

#include "fmt/chrono.h"

G4Run* RMGRunAction::GenerateRun() {
  fRMGRun = new RMGRun();
  return fRMGRun;
}

RMGRunAction::RMGRunAction(bool persistency) :
  fIsPersistencyEnabled(persistency) {}

RMGRunAction::RMGRunAction(RMGMasterGenerator* gene, bool persistency) :
  fIsPersistencyEnabled(persistency),
  fRMGMasterGenerator(gene) {
}

void RMGRunAction::BeginOfRunAction(const G4Run*) {

  RMGLog::OutDev(RMGLog::debug, "Start of run action");

  if (fRMGMasterGenerator) {
    if (fRMGMasterGenerator->GetGenerator()) {
      fRMGMasterGenerator->GetGenerator()->BeginOfRunAction(fRMGRun);
    }
  }

  // save start time for future
  fRMGRun->SetStartTime(std::chrono::system_clock::now());

  if (this->IsMaster()) {
    auto tt = fmt::localtime(fRMGRun->GetStartTime());

    RMGLog::OutFormat(RMGLog::summary, "Starting run nr. {:d}. Current local time is {:%d-%m-%Y %H:%M:%S}",
        fRMGRun->GetRunID(), tt);
    RMGLog::OutFormat(RMGLog::summary, "Number of events to be processed: {:d}",
        fRMGRun->GetNumberOfEventToBeProcessed());
  }

  auto manager = RMGManager::GetRMGManager();
  auto g4manager = G4RunManager::GetRunManager();
  auto tot_events = g4manager->GetNumberOfEventsToBeProcessed();
  if (manager->GetPrintModulo() <= 0 and tot_events >= 100) manager->SetPrintModulo(tot_events/10);
  else if (tot_events < 100) manager->SetPrintModulo(100);
}

void RMGRunAction::EndOfRunAction(const G4Run*) {

  RMGLog::OutDev(RMGLog::debug, "End of run action");

  if (fRMGMasterGenerator) {
    if (fRMGMasterGenerator->GetGenerator()) {
      fRMGMasterGenerator->GetGenerator()->EndOfRunAction(fRMGRun);
    }
  }

  if (this->IsMaster()) {
    auto time_now = std::chrono::system_clock::now();

    RMGLog::OutFormat(RMGLog::summary, "Run nr. {:d} completed. {:d} events simulated. Current local time is {:%d-%m-%Y %H:%M:%S}",
        fRMGRun->GetRunID(), fRMGRun->GetNumberOfEventToBeProcessed(), fmt::localtime(time_now));

    auto start_time = fRMGRun->GetStartTime();
    auto tot_elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(time_now - start_time).count();
    long partial = 0;
    long elapsed_d = (tot_elapsed_s - partial) / 86400; partial += elapsed_d * 86400;
    long elapsed_h = (tot_elapsed_s - partial) / 3600;  partial += elapsed_h * 3600;
    long elapsed_m = (tot_elapsed_s - partial) / 60;    partial += elapsed_m * 60;
    long elapsed_s = tot_elapsed_s - partial;

    RMGLog::OutFormat(RMGLog::summary, "Stats: run time was {:d} days, {:d} hours, {:d} minutes and {:d} seconds",
        elapsed_d, elapsed_h, elapsed_m, elapsed_s);

    auto total_sec_hres = std::chrono::duration<double>(time_now - fRMGRun->GetStartTime()).count();

    double n_ev = fRMGRun->GetNumberOfEvent();
    RMGLog::OutFormat(RMGLog::summary, "Stats: average event processing time was {:.5g} seconds/event = {:.5g} events/second",
        total_sec_hres/n_ev, n_ev/total_sec_hres);

    if (n_ev < 100) RMGLog::Out(RMGLog::warning, "Event processing time might be inaccurate");
  }

  // reset print modulo
  // TODO: if it's user specified, it shouldn't be reset
  RMGManager::GetRMGManager()->SetPrintModulo(-1);

}

// vim: tabstop=2 shiftwidth=2 expandtab

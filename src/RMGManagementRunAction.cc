#include "RMGManagementRunAction.hh"

#include <limits>
#include <ctime>
#include <cstdlib>

#include "G4Run.hh"
#include "G4RunManager.hh"

#include "RMGRun.hh"
#include "RMGLog.hh"
#include "RMGVOutputManager.hh"
#include "RMGManager.hh"
#include "RMGGeneratorPrimary.hh"
#include "RMGVGenerator.hh"
#include "RMGManagementEventAction.hh"

#include "fmt/chrono.h"

G4Run* RMGManagementRunAction::GenerateRun() {
  fRMGRun = new RMGRun();
  return fRMGRun;
}

RMGManagementRunAction::RMGManagementRunAction(RMGGeneratorPrimary* gene) {
  fRMGGeneratorPrimary = gene;
}

void RMGManagementRunAction::BeginOfRunAction(const G4Run*) {

  RMGLog::OutDev(RMGLog::debug, "Start of run action");

  if (fRMGGeneratorPrimary) {
    if (fRMGGeneratorPrimary->GetGenerator()) {
      fRMGGeneratorPrimary->GetGenerator()->BeginOfRunAction(fRMGRun);
    }
  }

  // if (manager->GetRMGEventAction()->GetOutputManager()) {
  //   manager->GetRMGEventAction()->GetOutputManager()->BeginOfRunAction();
  // }
  // else RMGLog::Out(RMGLog::warning, "No Output specified!");

  if (this->IsMaster()) {
    // save start time for future
    fRMGRun->SetStartTime(std::chrono::system_clock::now());
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

void RMGManagementRunAction::EndOfRunAction(const G4Run*) {

  RMGLog::OutDev(RMGLog::debug, "End of run action");

  if (fRMGGeneratorPrimary) {
    if (fRMGGeneratorPrimary->GetGenerator()) {
      fRMGGeneratorPrimary->GetGenerator()->EndOfRunAction(fRMGRun);
    }
  }

  // if (manager->GetRMGEventAction()->GetOutputManager()) {
  //   manager->GetRMGEventAction()->GetOutputManager()->EndOfRunAction();
  // }

  if (this->IsMaster()) {
    auto time_now = std::chrono::system_clock::now();

    RMGLog::OutFormat(RMGLog::summary, "Run nr. {:d} completed. {:d} events simulated. Current local time is {:%d-%m-%Y %H:%M:%S}",
        fRMGRun->GetRunID(), fRMGRun->GetNumberOfEventToBeProcessed(), fmt::localtime(time_now));

    auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(time_now - fRMGRun->GetStartTime()).count();

    auto start_time = fRMGRun->GetStartTime();
    auto tot_elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(time_now - start_time).count();
    long partial = 0;
    long elapsed_d = (tot_elapsed_s - partial) / 86400; partial += elapsed_d * 86400;
    long elapsed_h = (tot_elapsed_s - partial) / 3600;  partial += elapsed_h * 3600;
    long elapsed_m = (tot_elapsed_s - partial) / 60;    partial += elapsed_m * 60;
    long elapsed_s = tot_elapsed_s - partial;

    RMGLog::OutFormat(RMGLog::summary, "Stats: run time was {:d} days, {:d} hours, {:d} minutes and {:d} seconds",
        elapsed_d, elapsed_h, elapsed_m, elapsed_s);

    RMGLog::OutFormat(RMGLog::summary, "Stats: average event processing time was {:.5g} seconds/event = {:.5g} events/second",
        total_sec*1./fRMGRun->GetNumberOfEvent(),
        fRMGRun->GetNumberOfEvent()*1./total_sec);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

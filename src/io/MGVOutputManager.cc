#include "G4GenericIon.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "MGVOutputManager.hh"
#include "MGLog.hh"

G4String  MGVOutputManager::fFileName = "";

MGVOutputManager::MGVOutputManager():
  fUseTimeWindow(false),
  fTimeWindow(1 * CLHEP::second),
  fOffsetTime(0 * CLHEP::second),
  fTempOffsetTime(0 * CLHEP::second),
  fHasRadDecay(true),
  fInNewStage(false),
  fOnFirstTrack(false),
  fUseImportanceSamplingWindow(false),
  fSchemaDefined(false),
  fWaveformsSaved(false) {}

/* This method returns true if the track is time windowed and false otherwise.
 * If fUseTimeWindow is true, then will check and see if RadioactiveDecay is a
 * valid process (first time called only), and then compare RD process pointer
 * with track's pointer to CreatorProcess.  If it satisfies that and GlobalTime
 * is greater than fTimeWindow, the global time is set to zero and the return
 * value is true.
 */
G4bool MGVOutputManager::IsTrackTimeWindowed(const G4Track* aTrack) {

  // return false if time windowing is not used or if radioactive decay is not
  // available for ions
  if (!fUseTimeWindow) return false;
  if (!fHasRadDecay) return false;

  // search for radioactive decay in the process list
  if (!fRadDecayProcPointer) {
    auto proc_list = G4GenericIon::GenericIon()->GetProcessManager()->GetProcessList();
    for (G4int k = 0; k < proc_list->size(); k++) {
      auto proc = (*proc_list)[k];
      if (proc->GetProcessName() == "RadioactiveDecay") {
        fRadDecayProcPointer = proc;
      }
    }
    if (!fRadDecayProcPointer) fHasRadDecay = false;
  }

  // test whether the track should be time windowed
  if (fRadDecayProcPointer and
      (aTrack->GetCreatorProcess() == fRadDecayProcPointer) and
      (aTrack->GetGlobalTime() > fTimeWindow)) {

    fTempOffsetTime = aTrack->GetGlobalTime();

    // This is BAD, but G4 made aTrack const, and we NEED to change it.
    // Other options would result in heinous code bloat.
    ((G4Track*) aTrack)->SetGlobalTime(0 * CLHEP::second);

    return true;
  }
  return false;
}

/* Return true if this track is windowed for importance sampling, false
 * otherwise.  If fUseImportanceSamplingWindow is true, this method will search
 * for ImportanceProcess the first time it is called.
 */
G4bool MGVOutputManager::IsTrackImportanceSamplingWindowed(const G4Track* aTrack) {

  // whether ImportanceProcess is in the process list
  static bool has_imp_proc = true;

  // pointer to ImportanceProcess
  static G4VProcess* imp_proc_ptr = nullptr;

  // return false if importance sampling windowing isn't used
  if (!fUseImportanceSamplingWindow) return false;

  if (!has_imp_proc) return false;

  // search for importance process pointer
  if (!imp_proc_ptr) {

    // loop over all particles
    // MGLog::O("searching for importance process");
    auto particle_tbl = G4ParticleTable::GetParticleTable();

    for (G4int ipart = 0; ipart < particle_tbl->entries(); ipart++) {

      // get name and process list for this particle
      auto particle = particle_tbl->GetParticle(ipart);
      auto particle_name = particle->GetParticleName();
      auto pmanager = particle->GetProcessManager();
      auto pvector = pmanager->GetProcessList();
      // MGLog(debugging) << "particle = " << particleName << endlog;

      // loop over processes for this particle
      for (G4int iproc = 0; iproc < pvector->size(); iproc++) {

        // get process name
        auto process = (*pvector)[iproc];
        auto proc_name = process->GetProcessName();
        // MGLog(debugging) << "\tprocess = " << proc_name << endlog;

        // test whether process name is ImportanceProcess
        if (proc_name == "ImportanceProcess") {
          // MGLog(routine)
          //   << "found importance process for particle="
          //   << particleName
          //   << endlog;

          imp_proc_ptr = process;
          // MGLog(routine) << "windowing tracks created by importance sampling" << endlog;

          break;  // exit process loop
        }
      }
      if ( imp_proc_ptr ) break;
    }
    // if ImportanceProcess pointer was not found, set has_imp_proc to false
    if (!imp_proc_ptr) has_imp_proc = false;
  }

  // The track should be time windowed if the creator process is
  // ImportanceProcess.  Test has_imp_proc in case it was modified above.
  if (has_imp_proc and (aTrack->GetCreatorProcess() == imp_proc_ptr)) return true;

  return false;
}

/*
Return true if this is the first track in a new stage; false otherwise.
*/
G4bool MGVOutputManager::IsTrackFirstInNewStage() {
  // test whether this is a new stage
  if (fInNewStage) {
    // test whether this is the first track in a new stage
    if (fOnFirstTrack) {
        fOnFirstTrack = false;
        return true;
    }
  }
  return false;
}

/*
This method classifies a new track.  IsTrackTimeWindowed() is called, and if
the track is windowed for time or importance sampling reasons, it is sent to
the "waiting" stack.
*/
G4ClassificationOfNewTrack MGVOutputManager::StackingAction(const G4Track* aTrack) {

  auto classification = fUrgent;

  // test whether both time windowing and importance sampling windowing are
  // used; output a warning if so
  static G4bool is_windowing_tested = false;
  if (!is_windowing_tested) {
    if (fUseTimeWindow and fUseImportanceSamplingWindow) {
      // MGLog(warning)
      //   << " *** Both time windowing and importance sampling windowing"
      //   << " are being used! ***" << endlog;
      // MGLog(warning)
      //   << " *** This may cause problems!! ***"
      //   << endlog;
    }
    is_windowing_tested = true;
  }

  /*
  If importance sampling windowing is used and this is the first track in a
  new stage (from the waiting stack) to the urgent stack.  The tracks in the
  waiting stack were sent there because they were created by
  ImportanceProcess; they must be sent to the urgent stack one at a time for
  processing.
  */
  if (fUseImportanceSamplingWindow && this->IsTrackFirstInNewStage()) {
    classification = fUrgent;
  }
  else { /* Otherwise, test whether the track should be sent to the waiting stack. */

    // determine whether track is time windowed
    G4bool is_track_time_windowed = this->IsTrackTimeWindowed(aTrack);

    // determine whether track is windowed for importance sampling
    G4bool is_track_importance_sampling_windowed = this->IsTrackImportanceSamplingWindowed(aTrack);

    // send track to waiting stack if it is windowed
    if (is_track_time_windowed or is_track_importance_sampling_windowed) {
      classification = fWaiting;
    }
  }

  // MGLog(debugging)
  //    << aTrack->GetDefinition()->GetParticleName()
  //    << " with Track ID = " << aTrack->GetTrackID() << " is sent to"
  //    << endlog;

  if (classification == fUrgent) {
    // MGLog(debugging)<<"\t urgent stack."<<endlog;
  }
  else if (classification == fWaiting) {
    // MGLog(debugging)<<"\t waiting stack."<<endlog;
  }

  return classification;
}

/*
When G4 asks the stack manager to pop off a new track and the urgent stack is
empty, it moves all the tracks from the waiting to the urgent stack, then this
method is called.
*/
void MGVOutputManager::NewStage() {

  // write and reset so the output is windowed
  if (fUseTimeWindow or fUseImportanceSamplingWindow) {

    // MGLog(debugging)
    //   << "There are "
    //   << G4EventManager::GetEventManager()->GetStackManager()->GetNTotalTrack()
    //   << " tracks in the waiting stack." << endlog;

    if (G4EventManager::GetEventManager()->GetStackManager()->GetNTotalTrack() != 0){

      if (fUseTimeWindow) fOffsetTime += fTempOffsetTime;

      this->WritePartialEvent(G4RunManager::GetRunManager()->GetCurrentEvent());
      this->ResetPartialEvent(G4RunManager::GetRunManager()->GetCurrentEvent());
    }
  }

  // when using importance sampling windowing, process one track created by
  // ImportanceProcess at a time
  if (fUseImportanceSamplingWindow) {

    fInNewStage = true;
    fOnFirstTrack = true;

    /*
    G4StackManager::ReClassify() moves all urgent tracks to a temporary stack,
    then pops them off one at a time calling StackingAction(), and then stacking
    them on the appropriate stack.
    */
    G4EventManager::GetEventManager()->GetStackManager()->ReClassify();
    fInNewStage = false;
  }
}

//This function is only called if fUseTimeWindow is set to true.
//If a chosen OutputManager doesn't have it's own version of
//WritePartialEvent(), then it isn't set up to use TimeWindowing and
//inherits this version and the accompanying error is given.
void MGVOutputManager::WritePartialEvent(const G4Event*) {
  static G4bool warn_given = false;
  if (!warn_given) {
    // MGLog(warning) << "UseTimeWindow flag has been set true, but the" << endlog;
    // MGLog(warning) << "chosen OutputManager isn't set up to use Time" << endlog;
    // MGLog(warning) << "Windowing.  Global times of recorded steps may" << endlog;
    // MGLog(warning) << "not make sense." << endlog;
    warn_given = true;
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

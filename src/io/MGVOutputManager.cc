#include "G4GenericIon.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "MGVOutputManager.hh"
#include "MGLog.hh"

using namespace CLHEP;

G4String  MGVOutputManager::fFileName = "";

MGVOutputManager::MGVOutputManager():
  fSchemaDefined(false), 
  fWaveformsSaved(false), 
  fUseTimeWindow(false), 
  fTimeWindow(1 * second), 
  fOffsetTime(0 * second), 
  fTempOffsetTime(0 * second),
  fHasRadDecay(true), 
  fRadDecayProcPointer(NULL),
  fInNewStage(false),
  fOnFirstTrack(false),
  fUseImportanceSamplingWindow(false)
{;}

MGVOutputManager::~MGVOutputManager()
{;}


G4bool MGVOutputManager::IsTrackTimeWindowed(const G4Track* aTrack)
{
  /*
  This method returns true if the track is time windowed and false otherwise.
  If fUseTimeWindow is true, then will check and see if RadioactiveDecay is a
  valid process (first time called only), and then compare RD process pointer
  with track's pointer to CreatorProcess.  If it satisfies that and GlobalTime
  is greater than fTimeWindow, the global time is set to zero and the return
  value is true.
  */

  // return false if time windowing is not used or if radioactive decay is not
  // available for ions
  if ( !fUseTimeWindow ) return false;
  if ( !fHasRadDecay ) return false;

  // search for radioactive decay in the process list
  if ( fRadDecayProcPointer == NULL ) {

     G4ProcessVector *theProcessList =
      G4GenericIon::GenericIon()->GetProcessManager()->GetProcessList();

      for(int k = 0; k < theProcessList->size(); k++){

        G4VProcess* theProcess = (*theProcessList)[k];

        if ( theProcess->GetProcessName() == "RadioactiveDecay" ){
          fRadDecayProcPointer = theProcess;
        }

      }  // end loop over theProcessList

      if ( fRadDecayProcPointer == NULL ) fHasRadDecay = false;

    }  // end test of fRadDecayProcPointer

  // test whether the track should be time windowed
  if ( fRadDecayProcPointer && 
    ( aTrack->GetCreatorProcess() == fRadDecayProcPointer) &&
    ( aTrack->GetGlobalTime() > fTimeWindow ) ) {

    fTempOffsetTime = aTrack->GetGlobalTime();

    // This is BAD, but G4 made aTrack const, and we NEED to change it.  
    // Other options would result in heinous code bloat.
    ((G4Track*) aTrack)->SetGlobalTime(0.0 * second);

    return true;

  } // end test of whether track should be windowed

  return false;

}

G4bool MGVOutputManager::IsTrackImportanceSamplingWindowed(const G4Track* aTrack)
{
  /*
  Return true if this track is windowed for importance sampling, false
  otherwise.  If fUseImportanceSamplingWindow is true, this method will search
  for ImportanceProcess the first time it is called.
  */

  // whether ImportanceProcess is in the process list
  static bool hasImpProcess = true;

  // pointer to ImportanceProcess
  static G4VProcess* importanceProcPointer = NULL;

  // return false if importance sampling windowing isn't used
  if ( !fUseImportanceSamplingWindow ) return false;

  if ( !hasImpProcess ) return false;

  // search for importance process pointer 
  if ( importanceProcPointer == NULL ) {

    // loop over all particles
    // MGLog::O("searching for importance process");
    G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

    for ( G4int iParticle = 0; iParticle < theParticleTable->entries(); iParticle++ ) {

      // get name and process list for this particle
      G4ParticleDefinition* particle = theParticleTable->GetParticle(iParticle);
      G4String particleName = particle->GetParticleName();
      G4ProcessManager* pmanager = particle->GetProcessManager();
      G4ProcessVector* pvector = pmanager->GetProcessList();
      // MGLog(debugging) << "particle = " << particleName << endlog;

      // loop over processes for this particle
      for ( G4int iProcess = 0; iProcess < pvector->size(); iProcess++) {

        // get process name
        G4VProcess* process = (*pvector)[iProcess];
        G4String processName = process->GetProcessName();
        // MGLog(debugging) << "\tprocess = " << processName << endlog;

        // test whether process name is ImportanceProcess
        if ( processName == "ImportanceProcess" ) {

          // MGLog(routine)
          //   << "found importance process for particle="
          //   << particleName
          //   << endlog;

          importanceProcPointer = process;
          // MGLog(routine) << "windowing tracks created by importance sampling" << endlog;

          break;  // exit process loop

        }  // end test of process name

      } // end loop over process vector

      if ( importanceProcPointer ) break;  // exit particle loop

    }  // end loop over particle table

    // if ImportanceProcess pointer was not found, set hasImpProcess to false
    if ( !importanceProcPointer ) hasImpProcess = false;

  }  // end test of importanceProcPointer


  // The track should be time windowed if the creator process is
  // ImportanceProcess.  Test hasImpProcess in case it was modified above.

  if ( hasImpProcess && 
    ( aTrack->GetCreatorProcess() == importanceProcPointer ) ) {

    return true;

  } 

  return false;

}


G4bool MGVOutputManager::IsTrackFirstInNewStage()
{
  /*
  Return true if this is the first track in a new stage; false otherwise.
  */

  // test whether this is a new stage
  if ( fInNewStage ) {
    
    // test whether this is the first track in a new stage
    if ( fOnFirstTrack ) {

        fOnFirstTrack = false;
        return true;
    }

  }

  return false;

}


G4ClassificationOfNewTrack MGVOutputManager::StackingAction(const G4Track* aTrack)
{
  /*
  This method classifies a new track.  IsTrackTimeWindowed() is called, and if
  the track is windowed for time or importance sampling reasons, it is sent to
  the "waiting" stack.
  */

  G4ClassificationOfNewTrack classification = fUrgent;

  // test whether both time windowing and importance sampling windowing are
  // used; output a warning if so
  static G4bool isWindowingTested = false;
  if ( !isWindowingTested ) {

    if ( fUseTimeWindow && fUseImportanceSamplingWindow ) {
      // MGLog(warning) 
      //   << " *** Both time windowing and importance sampling windowing"
      //   << " are being used! ***" << endlog;
      // MGLog(warning) 
      //   << " *** This may cause problems!! ***"
      //   << endlog;
    }

    isWindowingTested = true;

  }  // end check of isWindowingTested


  if ( fUseImportanceSamplingWindow && IsTrackFirstInNewStage() ) {

    /* 
    If importance sampling windowing is used and this is the first track in a
    new stage (from the waiting stack) to the urgent stack.  The tracks in the
    waiting stack were sent there because they were created by
    ImportanceProcess; they must be sent to the urgent stack one at a time for
    processing.
    */

    classification = fUrgent; 

  } else { 

    /*
    Otherwise, test whether the track should be sent to the waiting stack.
    */
  
    // determine whether track is time windowed
    G4bool isTrackTimeWindowed = IsTrackTimeWindowed(aTrack);

    // determine whether track is windowed for importance sampling
    G4bool isTrackImportanceSamplingWindowed =
      IsTrackImportanceSamplingWindowed(aTrack);
   
    // send track to waiting stack if it is windowed
    if ( isTrackTimeWindowed || isTrackImportanceSamplingWindowed ) {
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

  // end debugging output

  return classification;
}


void MGVOutputManager::NewStage()
{
  /*
  When G4 asks the stack manager to pop off a new track and the urgent stack is
  empty, it moves all the tracks from the waiting to the urgent stack, then this
  method is called.
  */

  // write and reset so the output is windowed
  if( fUseTimeWindow || fUseImportanceSamplingWindow ) {

    // MGLog(debugging)
    //   << "There are "
    //   << G4EventManager::GetEventManager()->GetStackManager()->GetNTotalTrack()
    //   << " tracks in the waiting stack." << endlog;

    if(G4EventManager::GetEventManager()->GetStackManager()->GetNTotalTrack() != 0){

      if ( fUseTimeWindow ) fOffsetTime += fTempOffsetTime;

      WritePartialEvent(G4RunManager::GetRunManager()->GetCurrentEvent());
      ResetPartialEvent(G4RunManager::GetRunManager()->GetCurrentEvent());

    }  // end check on number of total tracks

  }  // end test of fUseTimeWindow || fUseImportanceSamplingWindow


  // when using importance sampling windowing, process one track created by
  // ImportanceProcess at a time
  if ( fUseImportanceSamplingWindow ) {

    fInNewStage = true;
    fOnFirstTrack = true;

    /*
    G4StackManager::ReClassify() moves all urgent tracks to a temporary stack,
    then pops them off one at a time calling StackingAction(), and then stacking
    them on the appropriate stack.
    */
    G4EventManager::GetEventManager()->GetStackManager()->ReClassify();

    fInNewStage = false;

  }  // end test of fUseImportanceSamplingWindow

}

 
void MGVOutputManager::WritePartialEvent(const G4Event*)
{
  //This function is only called if fUseTimeWindow is set to true.
  //If a chosen OutputManager doesn't have it's own version of
  //WritePartialEvent(), then it isn't set up to use TimeWindowing and
  //inherits this version and the accompanying error is given.
  static G4bool WarningGiven = false;
  if(!WarningGiven){
    // MGLog(warning) << "UseTimeWindow flag has been set true, but the" << endlog;
    // MGLog(warning) << "chosen OutputManager isn't set up to use Time" << endlog;
    // MGLog(warning) << "Windowing.  Global times of recorded steps may" << endlog;
    // MGLog(warning) << "not make sense." << endlog;
    WarningGiven = true;
  }
}


// vim: tabstop=2 shiftwidth=2 expandtab

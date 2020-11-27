#ifndef _MGVOUTPUTMANAGER_HH_
#define _MGVOUTPUTMANAGER_HH_

#include "globals.hh"
#include "G4ClassificationOfNewTrack.hh"

class G4Event;
class G4Track;
class G4Step;
class G4SteppingManager;
class G4VProcess;
class MGVOutputManager {

  public:

    MGVOutputManager();
    virtual ~MGVOutputManager() = default;

    MGVOutputManager           (MGVOutputManager const&) = delete;
    MGVOutputManager& operator=(MGVOutputManager const&) = delete;
    MGVOutputManager           (MGVOutputManager&&)      = delete;
    MGVOutputManager& operator=(MGVOutputManager&&)      = delete;

    /** Action to perform at beginning and end of events and runs
     * Detector specific.
     * Only include actions that directly affect the Root tree.
     * Other actions should go into the MGManagementEventAction class.
     */
    virtual void BeginOfEventAction(const G4Event*);
    virtual void BeginOfRunAction();
    virtual void EndOfEventAction(const G4Event*);
    virtual void EndOfRunAction();
    virtual void SteppingAction(const G4Step*, G4SteppingManager*);

    /// Whether track is time windowed; sets track global time to 0 if true.
    virtual G4bool IsTrackTimeWindowed(const G4Track* aTrack);

    /// Whether track is windowed for importance sampling
    virtual G4bool IsTrackImportanceSamplingWindowed(const G4Track* aTrack);

    /// Whether track is first in a new stage
    virtual G4bool IsTrackFirstInNewStage();

    /// Track classification, called by G4UserStackingAction::ClassifyNewTrack()
    virtual G4ClassificationOfNewTrack StackingAction(const G4Track* aTrack);

    /**
     * When urgent stack is empty, all tracks are moved from waiting to urgent
     * and this method is called.
     */
    /// Called after tracks are moved from waiting to urgent stack
    virtual void NewStage();

    virtual void PrepareNewEvent(const G4Event* = nullptr);
    virtual void WritePartialEvent(const G4Event*);
    virtual void ResetPartialEvent(const G4Event*);
    virtual void PreUserTrackingAction(const G4Track*);
    virtual void PostUserTrackingAction(const G4Track*);

    //Define detector specific tree schema
    //TODO: MUST defined in the derived class, make explicit
    virtual void DefineSchema();
    virtual void OpenFile();
    virtual void CloseFile();
    // allow for one to write out files in the midst of a run
    // (to save data being trashed by aborts)
    // By default, does nothing.
    virtual void WriteFile();

    // getters
    inline G4String GetFileName() { return fFileName; }
    inline G4double GetTimeWindow() { return fTimeWindow; }
    inline G4double GetOffsetTime() { return fOffsetTime; }
    inline G4bool   GetUseTimeWindow() { return fUseTimeWindow; }
    inline G4bool   GetWaveformsSaved() { return fWaveformsSaved; }
    inline G4bool   GetSchemaDefined() { return fSchemaDefined; }
    inline G4bool   GetUseImportanceSamplingWindow() { return fUseImportanceSamplingWindow; }

    // setters
    void SetFileName(G4String& name) { fFileName = name; }
    void SetSchemaDefined(G4bool sta) { fSchemaDefined = sta; }
    void SetWaveformsSaved(G4bool saved) { fWaveformsSaved = saved; }
    void SetUseTimeWindow(G4bool val) { fUseTimeWindow = val; }
    void SetTimeWindow(G4double val) { fTimeWindow = val; }
    void SetOffsetTime(G4double val) { fOffsetTime = val; }
    void SetUseImportanceSamplingWindow(G4bool val) { fUseImportanceSamplingWindow = val; }

  protected:

    static G4String fFileName;
    G4bool      fUseTimeWindow;               // if true will enable time windowing
    G4double    fTimeWindow;                  // Time Window used in Windowing.
    G4double    fOffsetTime;                  // Holds the cumulative deleted time for a track
    G4double    fTempOffsetTime;              // Holds the most recent deleted time for a track
    G4bool      fHasRadDecay;                 // bool to see if RadioactiveDK is valid process
    G4VProcess* fRadDecayProcPointer;         // pointer to Radioactive Decay Process
    G4bool      fInNewStage;                  // whether this is a new stage
    G4bool      fOnFirstTrack;                // whether this is the first track
    G4bool      fUseImportanceSamplingWindow; // whether to use importance sampling windowing

  private:

    G4bool fSchemaDefined;  // true if DefineSchema() has been run
    G4bool fWaveformsSaved; // is waveform simulation data being saved?
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

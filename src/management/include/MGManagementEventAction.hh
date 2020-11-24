#ifndef _MJMANAGEMENTEVENTACTION_HH_
#define _MJMANAGEMENTEVENTACTION_HH_

#include "globals.hh"
#include "G4Event.hh"
#include "G4UserEventAction.hh"

class MGManagementEventActionMessenger;
class MGVOutputManager;
class MGManagementEventAction : public G4UserEventAction {

  public:

    MGManagementEventAction();
    ~MGManagementEventAction();

    MGManagementEventAction           (MGManagementEventAction const&) = delete;
    MGManagementEventAction& operator=(MGManagementEventAction const&) = delete;
    MGManagementEventAction           (MGManagementEventAction&&)      = delete;
    MGManagementEventAction& operator=(MGManagementEventAction&&)      = delete;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

  /**
   * Set/Get output pointer and name
   */
  inline void         SetOutputManager(MGVOutputManager *outr) {fOutputManager=outr;}
  inline MGVOutputManager* GetOutputManager() {return fOutputManager;}
  inline const MGVOutputManager* GetConstOutputManager() const {return fOutputManager;}
  inline void         SetOutputName(const G4String name) {fOutputName = name;}
  inline G4String     GetOutputName() {return fOutputName;}
  inline void         SetReportingFrequency(G4int freq) { fReportingFrequency = freq;}
  inline G4int        GetReportingFrequency(){return fReportingFrequency;}
  inline void         SetWriteOutFileDuringRun(G4bool theBool) {fWriteOutFileDuringRun = theBool;}
  inline void         SetWriteOutFrequency(G4int theFreq) {fWriteOutFrequency = theFreq;}
  inline G4int        GetWriteOutFrequency() {return fWriteOutFrequency;}

  private:

  /**
   * Messenger class for user interface.
   */
  MGManagementEventActionMessenger* fG4Messenger;

  /**
   * Pointer to the output class. Set via User interface
   */
  MGVOutputManager* fOutputManager;

  /**
   * Name of Root output schema (as selected by user)
   */
  G4String fOutputName;

  G4int fReportingFrequency;  // Dump event # to output every fReportingFrequency events.
  // set up possibility to write out file during a run (after a number of events)
  G4int fWriteOutFrequency;
  G4bool fWriteOutFileDuringRun;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

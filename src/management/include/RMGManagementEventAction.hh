#ifndef _RMG_MANAGEMENT_EVENT_ACTION_HH_
#define _RMG_MANAGEMENT_EVENT_ACTION_HH_

#include "globals.hh"
#include "G4Event.hh"
#include "G4UserEventAction.hh"

class RMGManagementEventActionMessenger;
class RMGVOutputManager;
class RMGManagementEventAction : public G4UserEventAction {

  public:

    RMGManagementEventAction();
    ~RMGManagementEventAction();

    RMGManagementEventAction           (RMGManagementEventAction const&) = delete;
    RMGManagementEventAction& operator=(RMGManagementEventAction const&) = delete;
    RMGManagementEventAction           (RMGManagementEventAction&&)      = delete;
    RMGManagementEventAction& operator=(RMGManagementEventAction&&)      = delete;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    inline void SetOutputManager(RMGVOutputManager* outr) { fOutputManager = outr; }
    inline void SetOutputName(const G4String name) { fOutputName = name; }
    inline void SetReportingFrequency(G4int freq) { fReportingFrequency = freq; }
    inline void SetWriteOutFileDuringRun(G4bool val) { fWriteOutFileDuringRun = val; }
    inline void SetWriteOutFrequency(G4int val) { fWriteOutFrequency = val; }

    inline RMGVOutputManager* GetOutputManager() { return fOutputManager; }
    inline G4String GetOutputName() { return fOutputName; }
    inline G4int GetReportingFrequency() { return fReportingFrequency; }
    inline G4int GetWriteOutFrequency() { return fWriteOutFrequency; }

  private:

  RMGManagementEventActionMessenger* fG4Messenger; ///> Messenger class for user interface.
  RMGVOutputManager* fOutputManager; ///> Pointer to the output class. Set via user interface
  G4String fOutputName; ///> Name of output schema (as selected by user)
  G4int fReportingFrequency;
  G4int fWriteOutFrequency;
  G4bool fWriteOutFileDuringRun;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

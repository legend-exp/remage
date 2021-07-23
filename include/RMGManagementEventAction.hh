#ifndef _RMG_MANAGEMENT_EVENT_ACTION_HH_
#define _RMG_MANAGEMENT_EVENT_ACTION_HH_

#include <memory>

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

    inline RMGVOutputManager* GetOutputManager() { return fOutputManager; }
    inline G4String GetOutputName() { return fOutputName; }

  private:

    std::unique_ptr<RMGManagementEventActionMessenger> fG4Messenger;
    RMGVOutputManager* fOutputManager; ///> Pointer to the output class. Set via user interface
    G4String fOutputName; ///> Name of output schema (as selected by user)
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

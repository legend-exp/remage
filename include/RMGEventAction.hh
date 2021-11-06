#ifndef _RMG_MANAGEMENT_EVENT_ACTION_HH_
#define _RMG_MANAGEMENT_EVENT_ACTION_HH_

#include <memory>

#include "globals.hh"
#include "G4Event.hh"
#include "G4UserEventAction.hh"

class G4GenericMessenger;
class RMGVOutputManager;
class RMGEventAction : public G4UserEventAction {

  public:

    RMGEventAction();
    ~RMGEventAction();

    RMGEventAction           (RMGEventAction const&) = delete;
    RMGEventAction& operator=(RMGEventAction const&) = delete;
    RMGEventAction           (RMGEventAction&&)      = delete;
    RMGEventAction& operator=(RMGEventAction&&)      = delete;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    // inline void SetOutputManager(RMGVOutputManager* outr) { fOutputManager = outr; }
    // inline void SetOutputName(const G4String name) { fOutputName = name; }

    // inline RMGVOutputManager* GetOutputManager() { return fOutputManager; }
    // inline G4String GetOutputName() { return fOutputName; }

  private:

    // RMGVOutputManager* fOutputManager; ///> Pointer to the output class. Set via user interface
    // G4String fOutputName; ///> Name of output schema (as selected by user)

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

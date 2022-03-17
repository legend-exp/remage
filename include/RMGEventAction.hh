#ifndef _RMG_MANAGEMENT_EVENT_ACTION_HH_
#define _RMG_MANAGEMENT_EVENT_ACTION_HH_

#include <memory>

#include "G4Event.hh"
#include "G4UserEventAction.hh"

class RMGRunAction;
class G4GenericMessenger;
class RMGVOutputManager;
class RMGEventAction : public G4UserEventAction {

  public:

    RMGEventAction(RMGRunAction*);
    inline ~RMGEventAction() = default;

    RMGEventAction           (RMGEventAction const&) = delete;
    RMGEventAction& operator=(RMGEventAction const&) = delete;
    RMGEventAction           (RMGEventAction&&)      = delete;
    RMGEventAction& operator=(RMGEventAction&&)      = delete;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();
    RMGRunAction* fRunAction = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

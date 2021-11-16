#ifndef _RMG_UI_CMD_STEP_LIMIT_HH_
#define _RMG_UI_CMD_STEP_LIMIT_HH_

#include "G4UIcommand.hh"

class RMGUIcmdStepLimit : public G4UIcommand {

  public:

    RMGUIcmdStepLimit(std::string command_path, G4UImessenger* messenger);
    double GetStepSize(std::string par_string);
    std::string GetParticleName(std::string par_string);
    std::string GetVolumeName(std::string par_string);
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

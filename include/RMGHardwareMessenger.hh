// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _RMG_HARDWARE_MESSENGER_HH_
#define _RMG_HARDWARE_MESSENGER_HH_

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UImessenger.hh"

/** @brief Class for handling custom messenger commands relating to @c RMGHardware. */
class RMGHardware;
class RMGHardwareMessenger : public G4UImessenger {

  public:

    /** @brief Constructor based on a @c RMGHardware pointer.
     *  @details This currently initialises the @c "/RMG/Geometry/RegisterDetector" and
     *  the @c "/RMG/Geometry/SetMaxStepSize" commands.
     *  @param hw Pointer to the @c RMGHardware instance.
     */
    RMGHardwareMessenger(RMGHardware* hw);
    ~RMGHardwareMessenger();

    /** @brief Set the values for command
     *  @param command the command to set parameters
     *  @param newValues the parameter values.
     */
    void SetNewValue(G4UIcommand* command, G4String newValues) override;

    /** @brief Define the commands to set the sensitive detectors. */
    void DefineRegisterDetector();

    /** @brief Define the commands to set the step limits. */
    void DefineStepLimits();

  private:

    RMGHardware* fHardware;
    G4UIcommand* fRegisterCmd = nullptr;
    G4UIcmdWithADoubleAndUnit* fStepLimitsCmd = nullptr;

    void RegisterDetectorCmd(const std::string& parameters);
    void StepLimitsCmd(const std::string& parameters);
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

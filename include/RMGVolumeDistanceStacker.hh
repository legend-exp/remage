// Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#ifndef _RMG_VOLUME_DISTANCE_STACKER_HH_
#define _RMG_VOLUME_DISTANCE_STACKER_HH_

#include <memory>
#include <optional>
#include <string>

#include "G4GenericMessenger.hh"

#include "RMGVOutputScheme.hh"

/** @brief Special scheme to stack electron/positron tracks created in a specific volume. */
class RMGVolumeDistanceStacker : public RMGVOutputScheme {

  public:

    RMGVolumeDistanceStacker();

    /** @brief Wraps @c G4UserStackingAction::StackingActionClassify
     *  @details This is used to classify all e-/e+ tracks as @c fWaiting if the conditions are met.
     */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    /** @brief Set the minimum distance to any other volume for this track to be stacked. */
    void SetVolumeSafety(double safety) { fVolumeSafety = safety; }

    /** @brief Add a volume name in which to stack e-/e+ tracks. */
    void AddVolumeName(std::string volume) { fVolumeNames.insert(volume); }

    /** @brief Enable or disable Germanium-only filtering for distance calculations.
     *  @details When enabled, surface distance checks only consider daughter volumes
     *  registered as Germanium detectors, potentially improving performance.
     */
    void SetDistanceCheckGermaniumOnly(bool enable);

  private:

    std::unique_ptr<G4GenericMessenger> fMessenger;
    void DefineCommands();

    double fVolumeSafety = -1;
    std::set<std::string> fVolumeNames;


#endif

// vim: tabstop=2 shiftwidth=2 expandtab

// Copyright (C) 2024 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

#ifndef _RMG_STACKING_ACTION_HH_
#define _RMG_STACKING_ACTION_HH_

#include "G4UserStackingAction.hh"

class G4Track;
class RMGRunAction;
/**
 * @brief Stacking action delegating track classification to the registered output schemes.
 *
 * Iterates over the output schemes attached to the owning @ref RMGRunAction so that they can,
 * for instance, defer secondary tracks to a later processing stage.
 */
class RMGStackingAction : public G4UserStackingAction {

  public:

    RMGStackingAction(RMGRunAction*);
    ~RMGStackingAction() = default;

    RMGStackingAction(RMGStackingAction const&) = delete;
    RMGStackingAction& operator=(RMGStackingAction const&) = delete;
    RMGStackingAction(RMGStackingAction&&) = delete;
    RMGStackingAction& operator=(RMGStackingAction&&) = delete;

    /**
     * @brief Classify a newly created track by consulting the registered output schemes.
     * @details Schemes may return a classification different from @c fUrgent to defer
     * processing to a later stage (e.g. for two-pass optical photon handling).
     */
    G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
    /** @brief Advance to the next stacking stage; called when the current stack is empty. */
    void NewStage() override;
    /** @brief Reset the stage counter before tracking a new event. */
    void PrepareNewEvent() override;

  private:

    int fStage = 0;
    RMGRunAction* fRunAction = nullptr;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

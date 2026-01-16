// Copyright (C) 2025 Manuel Huber
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

#ifndef _RMG_PRIMARY_TRANSFORMER_HH_
#define _RMG_PRIMARY_TRANSFORMER_HH_

#include "G4PrimaryTransformer.hh"

class RMGPrimaryTransformer : public G4PrimaryTransformer {
  public:

    RMGPrimaryTransformer() : G4PrimaryTransformer() {
      // this suppresses the ZeroPolarization warning emitted for optical photons.
      nWarn = 11;
    };
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

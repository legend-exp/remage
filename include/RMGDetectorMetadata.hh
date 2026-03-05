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

#ifndef _RMG_DETECTOR_METADATA_HH_
#define _RMG_DETECTOR_METADATA_HH_

#include <string>

enum RMGDetectorType {
  kGermanium,
  kOptical,
  kScintillator,
  kCalorimeter,
};

struct RMGDetectorMetadata {
    /** @brief detector type */
    RMGDetectorType type;
    /** @brief detector (unique) identifier */
    int uid;
    /** @brief name of the referenced physical volume */
    std::string name;
    /** @brief copy number of the referenced physical volume */
    int copy_nr;
    /** @brief ntuple name override or empty string */
    std::string ntuple_name;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

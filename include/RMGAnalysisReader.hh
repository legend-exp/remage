// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
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

#ifndef _RMG_ANALYSIS_READER_HH_
#define _RMG_ANALYSIS_READER_HH_

#include <string>

#include "G4ThreeVector.hh"
#include "G4VAnalysisReader.hh"

#include "RMGVVertexGenerator.hh"

class RMGAnalysisReader {

  public:

    RMGAnalysisReader() = default;
    ~RMGAnalysisReader() = default;

    RMGAnalysisReader(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader(RMGAnalysisReader&&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader&&) = delete;

    bool OpenFile(std::string& name, std::string ntuple_dir_name, std::string ntuple_name);
    void CloseFile();

    [[nodiscard]] inline auto GetReader() { return fReader; }
    [[nodiscard]] inline auto GetNtupleId() const { return fNtupleId; }
    [[nodiscard]] inline auto& GetFileName() const { return fFileName; }

  private:

    G4VAnalysisReader* fReader = nullptr;
    int fNtupleId = -1;

    std::string fFileName;
    bool fFileIsTemp = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

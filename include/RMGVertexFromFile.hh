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

#ifndef _RMG_VERTEX_FROM_FILE_HH_
#define _RMG_VERTEX_FROM_FILE_HH_

#include <memory>
#include <string>

#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

#include "RMGAnalysisReader.hh"
#include "RMGVVertexGenerator.hh"

class G4VAnalysisReader;
class RMGVertexFromFile : public RMGVVertexGenerator {

  public:

    RMGVertexFromFile();
    ~RMGVertexFromFile() = default;

    RMGVertexFromFile(RMGVertexFromFile const&) = delete;
    RMGVertexFromFile& operator=(RMGVertexFromFile const&) = delete;
    RMGVertexFromFile(RMGVertexFromFile&&) = delete;
    RMGVertexFromFile& operator=(RMGVertexFromFile&&) = delete;

    bool GenerateVertex(G4ThreeVector&) override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void OpenFile(std::string& name);

  private:

    static G4Mutex fMutex;
    static RMGAnalysisReader* fReader;

    inline static double fXpos = NAN, fYpos = NAN, fZpos = NAN;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    std::string fNtupleDirectoryName = "vtx";
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

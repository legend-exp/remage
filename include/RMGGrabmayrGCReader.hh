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

#ifndef _RMG_GRABMAYR_GC_READER_HH_
#define _RMG_GRABMAYR_GC_READER_HH_

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "G4GenericMessenger.hh"
#include "globals.hh"

// Modified from WLGDPetersGammaCascadeReader originally contributed by Moritz Neuberger
struct GammaCascadeLine {
    G4int en;              // neutron energy [keV]
    G4int ex;              // excitation energy [keV]
    G4int m;               // multiplicity of gamma cascade
    G4int em;              // missing energy [keV]
    std::vector<G4int> eg; // energies of photons [keV]
};


class RMGGrabmayrGCReader {
  public:

    static G4ThreadLocal RMGGrabmayrGCReader* GetInstance();
    ~RMGGrabmayrGCReader();
    // RMGGrabmayrGCReader& operator=(const RMGGrabmayrGCReader&) = delete;

    G4bool IsApplicable(G4int a, G4int z);
    void CloseFiles();

    GammaCascadeLine GetNextEntry(G4int z, G4int a);

  private:

    static G4ThreadLocal RMGGrabmayrGCReader* instance;
    RMGGrabmayrGCReader();
    // std::vector<std::unique_ptr<std::ifstream>> files;
    //  map holding the corresponding file for each isotope
    std::map<std::pair<G4int, G4int>, std::unique_ptr<std::ifstream>> fCascadeFiles;
    std::unique_ptr<G4GenericMessenger> fGenericMessenger;
    G4int fGammaCascadeRandomStartLocation = 0;

    void SetGammaCascadeFile(const G4int z, const G4int a, const G4String file_name);
    void SetGammaCascadeRandomStartLocation(const int answer);
    void SetStartLocation(std::ifstream& file);

    void RandomizeFiles();
    void DefineCommands();

    // Nested messenger class
    class GCMessenger : public G4UImessenger {
      public:

        GCMessenger(RMGGrabmayrGCReader* reader);
        ~GCMessenger();

        void SetNewValue(G4UIcommand* command, G4String newValues) override;

      private:

        RMGGrabmayrGCReader* fReader;
        G4UIcommand* fGammaFileCmd;

        void GammaFileCmd(const std::string& parameters);
    };

    std::unique_ptr<GCMessenger> fUIMessenger;
};


#endif

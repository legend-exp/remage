// Copyright (C) 2024 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
// Copyright (C) 2024 Eric Esch <https://orcid.org/0009-0000-4920-9313>
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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "G4GenericMessenger.hh"
#include "globals.hh"

// Modified from WLGDPetersGammaCascadeReader originally contributed by Moritz Neuberger
/**
 * @brief One pre-computed neutron-capture gamma cascade.
 *
 * All energies are in keV. @c em is the energy not accounted for by the listed @c eg
 * photons (e.g. carried away by internal conversion electrons or unresolved low-energy
 * transitions). @ref RMGNeutronCaptureProcess assigns it as the initial nuclear excitation
 * energy of the recoil ion, so it is subsequently released by the Geant4 de-excitation model.
 */
struct GammaCascadeLine {
    G4int en;              ///< Neutron kinetic energy bin [keV].
    G4int ex;              ///< Capture-state excitation energy [keV].
    G4int m;               ///< Cascade multiplicity (number of @c eg entries).
    G4int em;              ///< Missing energy not carried by the listed photons [keV].
    std::vector<G4int> eg; ///< Photon energies of the cascade [keV].
};


/**
 * @brief One cascade file valid for a neutron kinetic-energy interval @c [en_low, en_high) keV.
 *
 * A given isotope @c (Z, A) may register several of these, each generated at a representative
 * neutron energy, tiling the neutron-energy axis. The right one is chosen at runtime from the
 * incoming neutron's kinetic energy. The stream is held by @c unique_ptr because
 * @c std::ifstream is non-copyable. Entries live in a @c std::vector, which only requires
 * move-constructibility.
 */
struct GammaCascadeFileEntry {
    G4double en_low;                     ///< Inclusive lower neutron KE bound [keV].
    G4double en_high;                    ///< Exclusive upper neutron KE bound [keV].
    std::unique_ptr<std::ifstream> file; ///< Open handle to the cascade file for this bin.
};


/**
 * @brief Thread-local reader of pre-computed (n,gamma) cascade tables.
 *
 * Loads per-isotope cascade files registered through messenger commands and serves them
 * to @ref RMGNeutronCaptureProcess one cascade at a time. The reader is a thread-local
 * singleton: every worker keeps its own file handles and its own read position.
 */
class RMGGrabmayrGCReader {
  public:

    /** @brief Thread-local singleton accessor. */
    static RMGGrabmayrGCReader* GetInstance();
    ~RMGGrabmayrGCReader();
    // RMGGrabmayrGCReader& operator=(const RMGGrabmayrGCReader&) = delete;

    /** @brief Whether a cascade file has been registered for the @c (Z, A) isotope. */
    G4bool IsApplicable(G4int z, G4int a);
    /** @brief Close all open cascade files held by this thread. */
    void CloseFiles();

    /**
     * @brief Read and return the next cascade entry for the @c (Z, A) isotope.
     * @details Selects the cascade file whose neutron kinetic-energy range contains
     * @c neutron_energy_keV (or the single registered file, if only one). If the energy falls
     * outside every registered range it is clamped to the nearest bin (a one-time warning is
     * emitted per isotope). The reader cycles back to the beginning when the file is exhausted.
     */
    GammaCascadeLine GetNextEntry(G4int z, G4int a, G4double neutron_energy_keV);

  private:

    static G4ThreadLocal RMGGrabmayrGCReader* instance;
    RMGGrabmayrGCReader();
    // map holding the cascade file(s) for each isotope, keyed by (Z, A). Each isotope may have
    // several files covering disjoint neutron kinetic-energy ranges (sorted ascending by en_low).
    std::map<std::pair<G4int, G4int>, std::vector<GammaCascadeFileEntry>> fCascadeFiles;
    std::set<std::pair<G4int, G4int>> fOutOfRangeWarned;
    std::unique_ptr<G4GenericMessenger> fGenericMessenger;
    G4int fGammaCascadeRandomStartLocation = 0;

    void SetGammaCascadeFile(G4int z, G4int a, G4String file_name);
    void SetGammaCascadeFilelist(G4int z, G4int a, G4String filelist_name);
    void RegisterCascadeFile(G4int z, G4int a, const G4String& file_name, G4double en_low, G4double en_high);
    void SetGammaCascadeRandomStartLocation(int answer);
    void SetStartLocation(std::ifstream& file) const;

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
        G4UIcommand* fGammaFilelistCmd;

        void GammaFileCmd(const std::string& parameters);
        void GammaFilelistCmd(const std::string& parameters);
    };

    std::unique_ptr<GCMessenger> fUIMessenger;
};


#endif

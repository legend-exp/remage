// Copyright (C) 2025 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
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

#ifndef _RMG_GEOMBENCH_HH_
#define _RMG_GEOMBENCH_HH_

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleGun.hh"
#include "G4VPhysicalVolume.hh"

#include "RMGGeomBenchOutputScheme.hh"
#include "RMGVGenerator.hh"

namespace u = CLHEP;

/**
 * @brief Primary generator that benchmarks geometry navigation on a regular 3D grid.
 *
 * Shoots geantinos from pixels of three orthogonal sampling planes covering the world
 * volume and measures how long the geometry navigator takes to step through each pixel.
 * Per-pixel timings are aggregated and written by @ref RMGGeomBenchOutputScheme.
 */
class RMGGeomBench : public RMGVGenerator {

  public:

    RMGGeomBench();
    ~RMGGeomBench();

    RMGGeomBench(RMGGeomBench const&) = delete;
    RMGGeomBench& operator=(RMGGeomBench const&) = delete;
    RMGGeomBench(RMGGeomBench&&) = delete;
    RMGGeomBench& operator=(RMGGeomBench&&) = delete;

    /** @brief Shoot a geantino from the pixel currently being benchmarked. */
    void GeneratePrimaries(G4Event* event) override;
    /** @brief No-op: vertex sampling is driven by the benchmark grid. */
    void SetParticlePosition(G4ThreeVector) override{};
    /** @brief Append a batch timing to the running median for the given pixel. */
    void RecordBatchTime(int pixel_idx, double batch_time);
    /** @brief Flush per-pixel median timings to the benchmark output scheme. */
    void SaveAllPixels();

    /** @brief Compute the sampling grid from the user-specified increments and widths. */
    void BeginOfRunAction(const G4Run* r) override;
    /** @brief Save aggregated timings to the output scheme. */
    void EndOfRunAction(const G4Run* r) override;

  private:

    // Helper to find the benchmark output scheme if it's active
    RMGGeomBenchOutputScheme* GetBenchmarkOutputScheme();

    std::unique_ptr<G4ParticleGun> fGun = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    int totalnevents;
    int totalnpixels;
    int neventsperpixel;

    // Configurable sampling parameters (user-specified increments)
    G4ThreeVector user_increment;
    G4ThreeVector sampling_width;

    // Calculated number of pixels based on increments and widths
    int npixels_x;
    int npixels_y;
    int npixels_z;
    size_t ID;

    double starttime;
    double currenttime;
    double bunchstarttime;

    // For tracking batches and calculating median
    std::vector<std::vector<double>> pixel_batch_times; // One vector of batch times per pixel
    int events_per_bunch;
    int total_batch_rounds;
    int current_batch_event;
    int current_pixel_index;
    int current_batch_round;

    G4ThreeVector origin;
    G4ThreeVector limit;
    G4ThreeVector increment;
};

#endif

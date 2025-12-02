// Copyright (C) 2025 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
#ifndef _RMG_GENERATOR_GEOMBENCH_HH_
#define _RMG_GENERATOR_GEOMBENCH_HH_

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

class RMGGeomBench : public RMGVGenerator {

  public:

    RMGGeomBench();
    ~RMGGeomBench();

    RMGGeomBench(RMGGeomBench const&) = delete;
    RMGGeomBench& operator=(RMGGeomBench const&) = delete;
    RMGGeomBench(RMGGeomBench&&) = delete;
    RMGGeomBench& operator=(RMGGeomBench&&) = delete;

    void GeneratePrimaries(G4Event* event) override;
    void SetParticlePosition(G4ThreeVector) override{};
    void RecordBatchTime(size_t pixel_idx, double batch_time);
    void SaveAllPixels();

    void BeginOfRunAction(const G4Run* r) override;
    void EndOfRunAction(const G4Run* r) override;

  private:

    // Helper to find the benchmark output scheme if it's active
    RMGGeomBenchOutputScheme* GetBenchmarkOutputScheme();

    std::unique_ptr<G4ParticleGun> fGun = nullptr;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();

    long totalnevents;
    size_t totalnpixels;
    int npixelsperrow;
    int neventsperpixel;
    double cubesize;

    // Configurable sampling parameters (user-specified increments)
    G4ThreeVector user_increment;
    G4ThreeVector sampling_width;

    // Calculated number of pixels based on increments and widths
    size_t npixels_x;
    size_t npixels_y;
    size_t npixels_z;
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

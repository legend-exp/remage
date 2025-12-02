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

<<<<<<< HEAD
  long   totalnevents;
  double totalnpixels;
  int npixelsperrow;
  double neventsperpixel;
  double cubesize;
  
  // Configurable sampling parameters (user-specified increments)
  double user_increment_x;
  double user_increment_y;
  double user_increment_z;
  double sampling_width_x;
  double sampling_width_y;
  double sampling_width_z;
  
  // Calculated number of pixels based on increments and widths
  int npixels_x;
  int npixels_y;
  int npixels_z;
  int ID;
  int whichntuple;
  G4ThreeVector trs;

  std::clock_t starttime;
  std::clock_t currenttime;
  std::clock_t bunchstarttime;

  // For tracking bunches and calculating median (stores CPU time in seconds)
  std::vector<double> bunch_times;
  int events_per_bunch;
  int current_event_in_pixel;
=======
    void GeneratePrimaries(G4Event* event) override;
<<<<<<< HEAD
    void SetParticlePosition(G4ThreeVector) override {};
<<<<<<< HEAD
    void SavePixel();
>>>>>>> c4d67577 (style: pre-commit fixes)
=======
=======
    void SetParticlePosition(G4ThreeVector) override{};
>>>>>>> 34c9dd85 (style: pre-commit fixes)
    void RecordBatchTime(size_t pixel_idx, double batch_time);
    void SaveAllPixels();
>>>>>>> f91278d3 (implemented better approach to be less suseptable for sudden spikes in calculation time)

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

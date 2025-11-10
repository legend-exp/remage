#ifndef _RMG_GENERATOR_BENCHMARK_HH_
#define _RMG_GENERATOR_BENCHMARK_HH_

#include <memory>
#include <string>
#include <vector>

#include "CLHEP/Units/SystemOfUnits.h"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4AnalysisManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

#include "RMGVGenerator.hh"
#include "RMGBenchmarkOutputScheme.hh"

namespace u = CLHEP;

class RMGGeneratorBenchmark : public RMGVGenerator {
  
public:
  
  RMGGeneratorBenchmark();
  ~RMGGeneratorBenchmark();
  
  RMGGeneratorBenchmark(RMGGeneratorBenchmark const&) = delete;
  RMGGeneratorBenchmark& operator=(RMGGeneratorBenchmark const&) = delete;
  RMGGeneratorBenchmark(RMGGeneratorBenchmark&&) = delete;
  RMGGeneratorBenchmark& operator=(RMGGeneratorBenchmark&&) = delete;
  
  void GeneratePrimaries(G4Event* event) override;
  void SetParticlePosition(G4ThreeVector) override {}
  void SavePixel();

  void PT(G4VPhysicalVolume* volume);
  
  void BeginOfRunAction(const G4Run* r) override;
  void EndOfRunAction(const G4Run* r) override;

private:
  
  // Helper to find the benchmark output scheme if it's active
  RMGBenchmarkOutputScheme* GetBenchmarkOutputScheme();

  std::unique_ptr<G4ParticleGun> fGun = nullptr;
  
  std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
  void DefineCommands();

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
  
  double starttime;
  double currenttime;
  double bunchstarttime;
  
  // For tracking 5% bunches and calculating median
  std::vector<double> bunch_times;
  int events_per_bunch;
  int current_event_in_pixel;

  double xorigin;
  double yorigin;
  double zorigin;

  double xlimit;
  double ylimit;
  double zlimit;

  double xcurrent;
  double ycurrent;
  double zcurrent;

  double increment_x;
  double increment_y;
  double increment_z;
  
  // Current pixel indices
  int pixel_x_index;
  int pixel_y_index;
  int pixel_z_index;
};

#endif
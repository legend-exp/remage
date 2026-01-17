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

#include "RMGGeomBench.hh"

#include <algorithm>
#include <cmath>
#include <ctime>

#include "G4AnalysisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleMomentum.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "G4TransportationManager.hh"
#include "G4VisExtent.hh"
#include "Randomize.hh"

#include "RMGGeomBenchOutputScheme.hh"
#include "RMGHardware.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGNavigationTools.hh"

namespace u = CLHEP;

RMGGeomBench::RMGGeomBench() : RMGVGenerator("Benchmark") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();

  // Set default values for configurable parameters
  // negative means auto-calculate (30 pixels default for increment, world bounds for width)

  totalnevents = totalnpixels = neventsperpixel = npixels_x = npixels_y = npixels_z = events_per_bunch = total_batch_rounds = current_batch_round = current_pixel_index = current_batch_event = 0;
  ID = 0;
  starttime = currenttime = bunchstarttime = 0.0;
  user_increment = sampling_width = origin = limit = increment = G4ThreeVector(-1.0, -1.0, -1.0);
}


RMGGeomBench::~RMGGeomBench() = default;

// Helper to find the benchmark output scheme if it's active
RMGGeomBenchOutputScheme* RMGGeomBench::GetBenchmarkOutputScheme() {
  auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  for (auto& scheme : det_cons->GetAllActiveOutputSchemes()) {
    auto benchmark_scheme = dynamic_cast<RMGGeomBenchOutputScheme*>(scheme.get());
    if (benchmark_scheme) return benchmark_scheme;
  }
  return nullptr;
}

void RMGGeomBench::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Generator/Benchmark/",
      "Commands for controlling the benchmarking simulation"
  );

  fMessenger->DeclarePropertyWithUnit("IncrementX", "mm", user_increment[0])
      .SetGuidance("Step size (increment) in X direction (negative = auto, default 30 pixels)")
      .SetParameterName("dx", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("IncrementY", "mm", user_increment[1])
      .SetGuidance("Step size (increment) in Y direction (negative = auto, default 30 pixels)")
      .SetParameterName("dy", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("IncrementZ", "mm", user_increment[2])
      .SetGuidance("Step size (increment) in Z direction (negative = auto, default 30 pixels)")
      .SetParameterName("dz", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("SamplingWidthX", "mm", sampling_width[0])
      .SetGuidance("Sampling width in X direction (negative = auto from world)")
      .SetParameterName("wx", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("SamplingWidthY", "mm", sampling_width[1])
      .SetGuidance("Sampling width in Y direction (negative = auto from world)")
      .SetParameterName("wy", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("SamplingWidthZ", "mm", sampling_width[2])
      .SetGuidance("Sampling width in Z direction (negative = auto from world)")
      .SetParameterName("wz", false)
      .SetDefaultValue("-1.0");
}


void RMGGeomBench::BeginOfRunAction(const G4Run* r) {

  // Things set up at the begin of run action:
  //  - Start time of entire run (as usual)
  //  - Number of events to be processed
  //  - The NTuples for storing positional and timing information
  //  - The positional limits for the generator
  //  - Making sure the user doesn't break the program

  // In this current implementation, these parameters are hardcoded.
  // Need to have a meeting with someone who understands the Python
  // wrapper in order to get these into the CLI.

  // All lengths in mm

  // NOTE: The benchmark output scheme is registered as an optional output scheme
  // in RMGUserInit::RegisterDefaultOptionalOutputSchemes() and must be activated
  // by the user with: /RMG/Output/ActivateOutputScheme Benchmark
  // It will be initialized by RMGRunAction::SetupAnalysisManager() if active.

  // whichntuple will be initialized to 0 on event 0
  // Start the timer
  starttime = std::clock();
  currenttime = std::clock();

  // Store total number of events to be processed
  totalnevents = r->GetNumberOfEventToBeProcessed();

  // The benchmarker assumes the world volume is a cube when calculating bounds
  auto worldsolid = G4TransportationManager::GetTransportationManager()
                        ->GetNavigatorForTracking()
                        ->GetWorldVolume()
                        ->GetLogicalVolume()
                        ->GetSolid();
  auto typeofworld = worldsolid->GetEntityType();
  if (typeofworld != "G4Box")
    RMGLog::OutFormat(
        RMGLog::warning,
        "World entity shape ({}) is not a G4Box (why not? :| ). A very large benchmarking cube may "
        "crash the simulation.",
        typeofworld
    );

  // set the origin and limit according to the world volume
  G4ThreeVector min, max;
  worldsolid->BoundingLimits(min, max);

  // Set up sampling dimensions based on user configuration or world bounds
  if (sampling_width.x() < 0) { sampling_width.setX(max.x() - min.x()); }
  if (sampling_width.y() < 0) { sampling_width.setY(max.y() - min.y()); }
  if (sampling_width.z() < 0) { sampling_width.setZ(max.z() - min.z()); }

  // Calculate increments and number of pixels
  // If user specified increment, use it; otherwise default to 30 pixels
  if (user_increment.x() > 0) {
    increment.setX(user_increment.x());
    npixels_x = static_cast<int>(std::ceil(sampling_width.x() / increment.x()));
  } else {
    npixels_x = 30; // default
    increment.setX(sampling_width.x() / npixels_x);
  }

  if (user_increment.y() > 0) {
    increment.setY(user_increment.y());
    npixels_y = static_cast<int>(std::ceil(sampling_width.y() / increment.y()));
  } else {
    npixels_y = 30; // default
    increment.setY(sampling_width.y() / npixels_y);
  }

  if (user_increment.z() > 0) {
    increment.setZ(user_increment.z());
    npixels_z = static_cast<int>(std::ceil(sampling_width.z() / increment.z()));
  } else {
    npixels_z = 30; // default
    increment.setZ(sampling_width.z() / npixels_z);
  }

  // Center the sampling region on the world volume center
  origin = (max + min) / 2.;
  limit = origin - sampling_width / 2.;


  RMGLog::Out(RMGLog::summary, "Benchmark sampling configuration:");
  RMGLog::OutFormat(
      RMGLog::summary,
      "  X: {} pixels, width = {} mm, increment = {} mm",
      npixels_x,
      sampling_width.x(),
      increment.x()
  );
  RMGLog::OutFormat(
      RMGLog::summary,
      "  Y: {} pixels, width = {} mm, increment = {} mm",
      npixels_y,
      sampling_width.y(),
      increment.y()
  );
  RMGLog::OutFormat(
      RMGLog::summary,
      "  Z: {} pixels, width = {} mm, increment = {} mm",
      npixels_z,
      sampling_width.z(),
      increment.z()
  );
  RMGLog::OutFormat(
      RMGLog::summary,
      "Sampling region origin at (X,Y,Z): ({}, {}, {}) mm",
      origin.x(),
      origin.y(),
      origin.z()
  );
  RMGLog::OutFormat(
      RMGLog::summary,
      "Sampling region lower limit at (X,Y,Z): ({}, {}, {}) mm",
      limit.x(),
      limit.y(),
      limit.z()
  );

  // Calculate the number of pixels to simulate, and number of primaries per pixel
  //  NOW that we know npixels_x, npixels_y, npixels_z

  // Total pixels across all three planes (XZ, YZ, XY)
  int pixels_xz = npixels_x * npixels_z;
  int pixels_yz = npixels_y * npixels_z;
  int pixels_xy = npixels_x * npixels_y;
  totalnpixels = pixels_xz + pixels_yz + pixels_xy;

  if (totalnpixels <= 0) {
    RMGLog::Out(RMGLog::fatal, "Total number of pixels is zero or negative! Exiting...");
  }

  neventsperpixel = totalnevents / totalnpixels;

  // Calculate events per bunch (1% of events per pixel)
  events_per_bunch = std::max(1, static_cast<int>(std::ceil(neventsperpixel * 0.01)));

  // Calculate total number of batch rounds
  total_batch_rounds = static_cast<int>(
      std::ceil(static_cast<double>(neventsperpixel) / events_per_bunch)
  );

  RMGLog::OutFormat(
      RMGLog::summary,
      "Total pixels to sample: {} (XZ: {}, YZ: {}, XY: {})",
      totalnpixels,
      pixels_xz,
      pixels_yz,
      pixels_xy
  );
  RMGLog::OutFormat(RMGLog::debug, "Events per bunch: {}", events_per_bunch);
  RMGLog::OutFormat(RMGLog::debug, "Total batch rounds: {}", total_batch_rounds);

  // Safeties

  // Could maybe downgrade this from a warning to a summary...
  if (neventsperpixel != floor(neventsperpixel)) {
    RMGLog::OutFormat(
        RMGLog::warning,
        "Specified number of primaries({}) doesn't divide evenly into the specified number of "
        "pixels ({})",
        totalnevents,
        totalnpixels
    );
    RMGLog::Out(RMGLog::warning, "Rounding down to nearest integer number of primaries per pixel.");
  }
  neventsperpixel = floor(neventsperpixel);

  // Having a single event sampled per pixel breaks the generator algorithm
  // Doing this would also be completely useless from a benchmarking perspective
  if (neventsperpixel < 2)
    RMGLog::Out(RMGLog::fatal, "Not enough primaries to sample each pixel at least twice! Exiting...");
  else RMGLog::OutFormat(RMGLog::summary, "Number of primaries per pixel: {}", neventsperpixel);

} // BeginOfRunAction


void RMGGeomBench::EndOfRunAction(const G4Run* /*r*/) {
  // Save the last batch if there's any data
  if (current_batch_event > 0) {
    double bunch_time = static_cast<double>(std::clock() - bunchstarttime) / CLOCKS_PER_SEC;
    RecordBatchTime(current_pixel_index, bunch_time);
    RMGLog::Out(RMGLog::debug, "Recorded final batch time in EndOfRunAction");
  }

  // Save all pixel data
  RMGLog::Out(RMGLog::debug, "Saving all pixel data in EndOfRunAction");
  SaveAllPixels();
} // EndOfRunAction


void RMGGeomBench::RecordBatchTime(int pixel_idx, double batch_time) {
  if (pixel_idx >= totalnpixels) {
    RMGLog::OutFormat(RMGLog::warning, "Invalid pixel index ({}) in RecordBatchTime", pixel_idx);
    return;
  }

  // Ensure the vector is large enough
  if (static_cast<int>(pixel_batch_times.size()) <= pixel_idx) {
    pixel_batch_times.resize(pixel_idx + 1);
  }

  pixel_batch_times[pixel_idx].push_back(batch_time);

  RMGLog::OutFormat(RMGLog::debug, "Recorded batch time {} s for pixel {}", batch_time, pixel_idx);
} // RecordBatchTime

void RMGGeomBench::SaveAllPixels() {
  // Find the benchmark output scheme if it's active
  auto benchmark_scheme = GetBenchmarkOutputScheme();
  if (!benchmark_scheme) {
    RMGLog::OutDev(
        RMGLog::warning,
        "Benchmark output scheme not active. Use /RMG/Output/ActivateOutputScheme Benchmark"
    );
    return;
  }

  // Iterate through all pixels and save their data
  int pixel_idx = 0;

  for (int plane = 0; plane < 3; plane++) {
    int max_i = 0, max_j = 0;

    switch (plane) {
      case 0: // XZ plane
        max_i = npixels_x;
        max_j = npixels_z;
        break;
      case 1: // YZ plane
        max_i = npixels_y;
        max_j = npixels_z;
        break;
      case 2: // XY plane
        max_i = npixels_x;
        max_j = npixels_y;
        break;
    }

    for (int j = 0; j < max_j; j++) {
      for (int i = 0; i < max_i; i++) {
        // Calculate position for this pixel
        double x_pos = 0.0, y_pos = 0.0, z_pos = 0.0;

        switch (plane) {
          case 0: // XZ plane
            x_pos = limit.x() + i * increment.x();
            y_pos = limit.y();
            z_pos = limit.z() + j * increment.z();
            break;
          case 1: // YZ plane
            x_pos = limit.x();
            y_pos = limit.y() + i * increment.y();
            z_pos = limit.z() + j * increment.z();
            break;
          case 2: // XY plane
            x_pos = limit.x() + i * increment.x();
            y_pos = limit.y() + j * increment.y();
            z_pos = limit.z();
            break;
        }

        // Calculate median time from batch times
        double median_time_per_event = 0.0;

        if (pixel_idx < static_cast<int>(pixel_batch_times.size()) &&
            !pixel_batch_times[pixel_idx].empty()) {
          std::vector<double> sorted_times = pixel_batch_times[pixel_idx];
          std::sort(sorted_times.begin(), sorted_times.end());

          size_t n = sorted_times.size();
          double median_time = 0.0;
          if (n % 2 == 0) {
            median_time = (sorted_times[n / 2 - 1] + sorted_times[n / 2]) / 2.0;
          } else {
            median_time = sorted_times[n / 2];
          }

          if (events_per_bunch > 0) { median_time_per_event = median_time / events_per_bunch; }

          RMGLog::OutFormat(
              RMGLog::debug,
              "Pixel {} (plane {}): {} batches, median time per event: {} s",
              pixel_idx,
              plane,
              n,
              median_time_per_event
          );
        }

        benchmark_scheme->SavePixel(plane, x_pos, y_pos, z_pos, median_time_per_event);
        pixel_idx++;
      }
    }
  }

  RMGLog::OutFormat(RMGLog::summary, "Saved data for all {} pixels", pixel_idx);
} // SaveAllPixels


void RMGGeomBench::GeneratePrimaries(G4Event* event) {

  ID = event->GetEventID();

  if (static_cast<int>(ID) >= neventsperpixel * totalnpixels)
    return; // Should only apply in situations where the nevents doesn't divide evenly into the npixels

  if (ID == 0) {
    // Originally initialized in BeginOfRunAction, but delay between that and evt 0 is non-trivial
    currenttime = std::clock();
    bunchstarttime = std::clock();

    current_batch_event = 0;
    current_pixel_index = 0;
    current_batch_round = 0;

    // Initialize storage for batch times
    pixel_batch_times.clear();
    pixel_batch_times.resize(totalnpixels);
  }

  // Calculate which batch round and which pixel we're in
  // Structure: batch_round -> pixel -> events within batch
  // Total events = total_batch_rounds * totalnpixels * events_per_bunch (approximately)
  int events_so_far = static_cast<int>(ID);
  current_batch_round = events_so_far / (totalnpixels * events_per_bunch);
  int remainder = events_so_far % (totalnpixels * events_per_bunch);
  current_pixel_index = remainder / events_per_bunch;
  current_batch_event = remainder % events_per_bunch;

  // Check if we're starting a new batch
  if (ID == 0 || current_batch_event == 0) {
    // Save the previous batch time if this isn't the first event
    if (ID > 0) {
      double bunch_time = static_cast<double>(std::clock() - bunchstarttime) / CLOCKS_PER_SEC;
      int prev_pixel_index = static_cast<int>(ID - 1) % (totalnpixels * events_per_bunch) /
                             events_per_bunch;
      RecordBatchTime(prev_pixel_index, bunch_time);
      RMGLog::OutFormat(
          RMGLog::debug,
          "Batch complete for pixel {} at event # {}, CPU time: {} s",
          prev_pixel_index,
          ID - 1,
          bunch_time
      );
    }

    // Start timing for new batch
    bunchstarttime = std::clock();

    if (current_batch_event == 0 && current_pixel_index == 0) {
      RMGLog::OutFormat(RMGLog::debug, "Starting batch round {} at event # {}", current_batch_round, ID);
    }
  }

  // Convert linear pixel index to plane and coordinates
  int pixels_xz = npixels_x * npixels_z;
  int pixels_yz = npixels_y * npixels_z;

  int plane = 0;
  int i_index = 0, j_index = 0;

  if (current_pixel_index < pixels_xz) {
    // XZ plane
    plane = 0;
    i_index = current_pixel_index % npixels_x;
    j_index = current_pixel_index / npixels_x;
  } else if (current_pixel_index < pixels_xz + pixels_yz) {
    // YZ plane
    plane = 1;
    int local_idx = current_pixel_index - pixels_xz;
    i_index = local_idx % npixels_y;
    j_index = local_idx / npixels_y;
  } else {
    // XY plane
    plane = 2;
    int local_idx = current_pixel_index - pixels_xz - pixels_yz;
    i_index = local_idx % npixels_x;
    j_index = local_idx / npixels_x;
  }

  // Calculate position for this pixel
  G4ThreeVector current_position;
  switch (plane) {
    case 0: // XZ plane
      current_position.setX(limit.x() + i_index * increment.x());
      current_position.setY(limit.y());
      current_position.setZ(limit.z() + j_index * increment.z());
      break;
    case 1: // YZ plane
      current_position.setX(limit.x());
      current_position.setY(limit.y() + i_index * increment.y());
      current_position.setZ(limit.z() + j_index * increment.z());
      break;
    case 2: // XY plane
      current_position.setX(limit.x() + i_index * increment.x());
      current_position.setY(limit.y() + j_index * increment.y());
      current_position.setZ(limit.z());
      break;
  }

  fGun->SetNumberOfParticles(1);
  fGun->SetParticleDefinition(G4Geantino::Definition());

  // Randomize particle position within the current grid space
  // For each face of the sampling cube, we will have two
  // randomized positional values, with the current position
  // as a lower bound, and one fixed positional value, which
  // is the face we're currently sampling from
  // To make the code prettier, I'll just sample each
  // value uniformly, then squish whichever one is constant

  double xtemp = G4UniformRand() * increment.x() + current_position.x();
  double ytemp = G4UniformRand() * increment.y() + current_position.y();
  double ztemp = G4UniformRand() * increment.z() + current_position.z();

  G4ThreeVector momentumdir(0., 0., 0.);

  switch (plane) {
    case 0: { // XZ
      ytemp = limit.y();
      momentumdir.setY(1.);
      break;
    }
    case 1: { // YZ
      xtemp = limit.x();
      momentumdir.setX(1.);
      break;
    }
    case 2: { // XY
      ztemp = limit.z();
      momentumdir.setZ(1.);
      break;
    }
    default: {
      RMGLog::OutFormat(RMGLog::fatal, "Invalid plane ({}) in GeneratePrimaries()", plane);
    }
  } // switch(plane)

  fGun->SetParticlePosition({xtemp, ytemp, ztemp});
  fGun->SetParticleMomentumDirection(momentumdir);
  fGun->SetParticleEnergy(1 * u::GeV);

  RMGLog::OutFormat(RMGLog::debug, "Sampled X: {}  Sampled Y: {}  Sampled Z: {}", xtemp, ytemp, ztemp);
  RMGLog::Out(RMGLog::debug, "Momentum direction: ", momentumdir);

  fGun->GeneratePrimaryVertex(event);

} // GeneratePrimaries

// vim: tabstop=2 shiftwidth=2 expandtab

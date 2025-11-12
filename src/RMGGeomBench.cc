// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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
  user_increment = G4ThreeVector(-1.0, -1.0, -1.0);
  sampling_width = G4ThreeVector(-1.0, -1.0, -1.0);
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

  // Initialize default geometry sampling parameters early to avoid
  // using uninitialized member variables (was causing undefined behaviour).
  cubesize = 10000.;
  npixelsperrow = 30; // Legacy variable, kept for backward compatibility

  origin = G4ThreeVector(0., 0., 0.);
  limit = origin - G4ThreeVector(cubesize / 2., cubesize / 2., cubesize / 2.);

  G4ThreeVector minn, maxx; // Only used for bounding conditions

  // Specify volume included in the benchmarking simulation (optional)
  std::string targetvolumename = "";

  // NOTE: The benchmark output scheme is registered as an optional output scheme
  // in RMGUserInit::RegisterDefaultOptionalOutputSchemes() and must be activated
  // by the user with: /RMG/Output/ActivateOutputScheme Benchmark
  // It will be initialized by RMGRunAction::SetupAnalysisManager() if active.

  // whichntuple will be initialized to 0 on event 0
<<<<<<< HEAD
<<<<<<< HEAD
  //Start the timer
  starttime   = std::clock();
  currenttime = std::clock();
  
  //Store total number of events to be processed
=======
  // Start the timer
  starttime = double(clock()) / 1000000.;
  currenttime = double(clock()) / 1000000.;

  // Store total number of events to be processed
>>>>>>> c4d67577 (style: pre-commit fixes)
=======
  // Start the timer
  starttime = std::clock();
  currenttime = std::clock();

  // Store total number of events to be processed
>>>>>>> ed62e2c2 (style: pre-commit fixes)
  totalnevents = r->GetNumberOfEventToBeProcessed();

  // The benchmarker assumes the world volume is a cube when calculating bounds
  auto worldsolid = G4TransportationManager::GetTransportationManager()
                        ->GetNavigatorForTracking()
                        ->GetWorldVolume()
                        ->GetLogicalVolume()
                        ->GetSolid();
  auto typeofworld = worldsolid->GetEntityType();
  if (typeofworld != "G4Box")
    RMGLog::Out(
        RMGLog::warning,
        "World entity shape (",
        typeofworld,
        ") is not a G4Box (why not? :| ). A very large benchmarking cube may crash the simulation."
    );

  // set the origin and limit according to the world volume
  G4ThreeVector min, max;
  worldsolid->BoundingLimits(min, max);

  // Set up sampling dimensions based on user configuration or world bounds
<<<<<<< HEAD:src/RMGGeneratorBenchmark.cc
<<<<<<< HEAD
<<<<<<< HEAD
  if (sampling_width_x < 0) {
    sampling_width_x = (max.x() - min.x());
  }
  if (sampling_width_y < 0) {
    sampling_width_y = (max.y() - min.y());
  }
  if (sampling_width_z < 0) {
    sampling_width_z = (max.z() - min.z());
  }
  
=======
  if (sampling_width_x < 0) { sampling_width_x = (max.x() - min.x()) * 1.25; }
  if (sampling_width_y < 0) { sampling_width_y = (max.y() - min.y()) * 1.25; }
  if (sampling_width_z < 0) { sampling_width_z = (max.z() - min.z()) * 1.25; }

>>>>>>> c4d67577 (style: pre-commit fixes)
=======
  if (sampling_width_x < 0) { sampling_width_x = (max.x() - min.x()); }
  if (sampling_width_y < 0) { sampling_width_y = (max.y() - min.y()); }
  if (sampling_width_z < 0) { sampling_width_z = (max.z() - min.z()); }
=======
  if (sampling_width.x() < 0) { sampling_width.setX(max.x() - min.x()); }
  if (sampling_width.y() < 0) { sampling_width.setY(max.y() - min.y()); }
  if (sampling_width.z() < 0) { sampling_width.setZ(max.z() - min.z()); }
>>>>>>> b1e137ca (Implementing suggestions):src/RMGGeomBench.cc

>>>>>>> ed62e2c2 (style: pre-commit fixes)
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
  RMGLog::Out(
      RMGLog::summary,
      "  X: ",
      npixels_x,
      " pixels, width = ",
      sampling_width.x(),
      " mm, increment = ",
      increment.x(),
      " mm"
  );
  RMGLog::Out(
      RMGLog::summary,
      "  Y: ",
      npixels_y,
      " pixels, width = ",
      sampling_width.y(),
      " mm, increment = ",
      increment.y(),
      " mm"
  );
  RMGLog::Out(
      RMGLog::summary,
      "  Z: ",
      npixels_z,
      " pixels, width = ",
      sampling_width.z(),
      " mm, increment = ",
      increment.z(),
      " mm"
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

  RMGLog::Out(
      RMGLog::summary,
      "Total pixels to sample: ",
      totalnpixels,
      " (XZ: ",
      pixels_xz,
      ", YZ: ",
      pixels_yz,
      ", XY: ",
      pixels_xy,
      ")"
  );
  RMGLog::Out(RMGLog::debug, "Events per 1% bunch: ", events_per_bunch);

  // Safeties

  // Could maybe downgrade this from a warning to a summary...
  if (neventsperpixel != floor(neventsperpixel)) {
    RMGLog::Out(
        RMGLog::warning,
        "Specified number of primaries(",
        totalnevents,
        ") doesn't divide evenly into the specified number of pixels (",
        totalnpixels,
        ")"
    );
    RMGLog::Out(RMGLog::warning, "Rounding down to nearest integer number of primaries per pixel.");
  }
  neventsperpixel = floor(neventsperpixel);

  // Having a single event sampled per pixel breaks the generator algorithm
  // Doing this would also be completely useless from a benchmarking perspective
  if (neventsperpixel < 2)
    RMGLog::Out(RMGLog::fatal, "Not enough primaries to sample each pixel at least twice! Exiting...");
  else RMGLog::Out(RMGLog::summary, "Number of primaries per pixel: ", neventsperpixel);

} // BeginOfRunAction


void RMGGeomBench::EndOfRunAction(const G4Run* /*r*/) {
  // Save the last pixel if there's any data
  if (!bunch_times.empty() || current_event_in_pixel > 0) {
    // Save any remaining partial bunch
    if (current_event_in_pixel % events_per_bunch != 0 && current_event_in_pixel > 0) {
<<<<<<< HEAD
      double bunch_time = static_cast<double>(std::clock() - bunchstarttime) / CLOCKS_PER_SEC;
=======
      double bunch_time = double(clock()) / 1000000. - bunchstarttime;
>>>>>>> c4d67577 (style: pre-commit fixes)
      bunch_times.push_back(bunch_time);
    }

    RMGLog::Out(RMGLog::debug, "Saving final pixel data in EndOfRunAction");
    SavePixel();
  }
} // EndOfRunAction


void RMGGeomBench::SavePixel() {
  // Find the benchmark output scheme if it's active
  auto benchmark_scheme = GetBenchmarkOutputScheme();
  if (!benchmark_scheme) {
    RMGLog::OutDev(
        RMGLog::warning,
        "Benchmark output scheme not active. Use /RMG/Output/ActivateOutputScheme Benchmark"
    );
    return;
  }

  if (whichntuple < 0 || whichntuple > 2) {
    RMGLog::Out(RMGLog::warning, "Invalid whichntuple (", whichntuple, ") in SavePixel(); skipping");
    return;
  }

  // Calculate median time from bunch_times
  double median_time = 0.0;
  if (!bunch_times.empty()) {
    std::vector<double> sorted_times = bunch_times;
    std::sort(sorted_times.begin(), sorted_times.end());

    size_t n = sorted_times.size();
    if (n % 2 == 0) {
      median_time = (sorted_times[n / 2 - 1] + sorted_times[n / 2]) / 2.0;
    } else {
      median_time = sorted_times[n / 2];
    }
<<<<<<< HEAD
<<<<<<< HEAD
    
=======

>>>>>>> ed62e2c2 (style: pre-commit fixes)
    RMGLog::Out(RMGLog::debug, "Pixel complete with ", n, " bunches. Median CPU time: ", median_time, " s");
  }

  double median_time_per_event = 0.0;
  if (events_per_bunch > 0) { median_time_per_event = median_time / events_per_bunch; }

  RMGLog::Out(
      RMGLog::debug,
      "Saving pixel data: Ntuple ",
      whichntuple,
      ", X: ",
      current_position.x(),
      ", Y: ",
      current_position.y(),
      ", Z: ",
      current_position.z(),
      ", Median CPU time of bunch: ",
      median_time,
      " s, Median CPU time per event: ",
      median_time_per_event,
      " s"
  );
<<<<<<< HEAD:src/RMGGeneratorBenchmark.cc
  benchmark_scheme->SavePixel(whichntuple, xcurrent, ycurrent, zcurrent, median_time_per_event);
<<<<<<< HEAD
  
=======
=======
  benchmark_scheme->SavePixel(whichntuple, current_position.x(), current_position.y(), current_position.z(), median_time_per_event);
>>>>>>> b1e137ca (Implementing suggestions):src/RMGGeomBench.cc

    RMGLog::Out(RMGLog::debug, "Pixel complete with ", n, " bunches. Median time: ", median_time, " s");
  }
  RMGLog::Out(
      RMGLog::debug,
      "Saving pixel data: Ntuple ",
      whichntuple,
      ", X: ",
      xcurrent,
      ", Y: ",
      ycurrent,
      ", Z: ",
      zcurrent,
      ", Median time: ",
      median_time,
      " s"
  );
  benchmark_scheme->SavePixel(whichntuple, xcurrent, ycurrent, zcurrent, median_time);

>>>>>>> c4d67577 (style: pre-commit fixes)
=======

>>>>>>> ed62e2c2 (style: pre-commit fixes)
  // Reset for next pixel
  bunch_times.clear();
  current_event_in_pixel = 0;
} // SavePixel


void RMGGeomBench::GeneratePrimaries(G4Event* event) {

  ID = event->GetEventID();

<<<<<<< HEAD
<<<<<<< HEAD
  if(ID >= neventsperpixel * totalnpixels)
    return;//Should only apply in situations where the nevents doesn't divide evenly into the npixels
  
  if(ID==0) {
    //Originally initialized in BeginOfRunAction, but delay between that and evt 0 is non-trivial
=======
  if (ID >= neventsperpixel * totalnpixels)
    return; // Should only apply in situations where the nevents doesn't divide evenly into the npixels

  if (ID == 0) {
    // Originally initialized in BeginOfRunAction, but delay between that and evt 0 is non-trivial
>>>>>>> ed62e2c2 (style: pre-commit fixes)
    currenttime = std::clock();
    bunchstarttime = std::clock();
=======
  if (ID >= neventsperpixel * totalnpixels)
    return; // Should only apply in situations where the nevents doesn't divide evenly into the npixels

  if (ID == 0) {
    // Originally initialized in BeginOfRunAction, but delay between that and evt 0 is non-trivial
    currenttime = double(clock()) / 1000000.;
    bunchstarttime = currenttime;
>>>>>>> c4d67577 (style: pre-commit fixes)
    current_event_in_pixel = 0;
    bunch_times.clear();

    // Initialize pixel indices and position
    whichntuple = 0;
    pixel_x_index = 0;
    pixel_y_index = 0;
    pixel_z_index = 0;
    // Initialize positions centered in the first pixel (consistent with later updates)
    current_position = limit + 0.5 * increment;
  }
<<<<<<< HEAD
<<<<<<< HEAD
  
=======

>>>>>>> ed62e2c2 (style: pre-commit fixes)
  // Check if we've completed a bunch
  if (current_event_in_pixel > 0 && current_event_in_pixel % events_per_bunch == 0) {
    double bunch_time = static_cast<double>(std::clock() - bunchstarttime) / CLOCKS_PER_SEC;
    bunch_times.push_back(bunch_time);
    bunchstarttime = std::clock();
    RMGLog::Out(RMGLog::debug, "Bunch complete at event # ", ID, ", CPU time: ", bunch_time, " s");
=======

  // Check if we've completed a 5% bunch
  if (current_event_in_pixel > 0 && current_event_in_pixel % events_per_bunch == 0) {
    double bunch_time = double(clock()) / 1000000. - bunchstarttime;
    bunch_times.push_back(bunch_time);
    bunchstarttime = double(clock()) / 1000000.;
    RMGLog::Out(RMGLog::debug, "Bunch complete at event # ", ID, ", time: ", bunch_time, " s");
>>>>>>> c4d67577 (style: pre-commit fixes)
  }

  bool pixel_just_completed = false;
  if ((ID + 1) % int(neventsperpixel) == 0) {
    // We'll be changing pixels, so save current data
    //  Save the last partial bunch if any events remain


    if (current_event_in_pixel % events_per_bunch != 0) {
<<<<<<< HEAD:src/RMGGeneratorBenchmark.cc
<<<<<<< HEAD
=======
      //RMGLog::Out(RMGLog::warning, "Partial bunch at pixel completion, event # ", ID, " with current event in pixel ", current_event_in_pixel);
>>>>>>> b1e137ca (Implementing suggestions):src/RMGGeomBench.cc
      double bunch_time = static_cast<double>(std::clock() - bunchstarttime) / CLOCKS_PER_SEC;
=======
      double bunch_time = double(clock()) / 1000000. - bunchstarttime;
>>>>>>> c4d67577 (style: pre-commit fixes)
      bunch_times.push_back(bunch_time);
    }

    RMGLog::Out(RMGLog::debug, "Current pixel complete at event # ", ID);
    SavePixel();
    pixel_just_completed = true;

    // Reset for next pixel and advance to next pixel
<<<<<<< HEAD
    bunchstarttime = std::clock();
=======
    bunchstarttime = double(clock()) / 1000000.;
>>>>>>> c4d67577 (style: pre-commit fixes)

    // Advance to the next pixel based on current plane
    if (whichntuple == 0) { // XZ plane
      pixel_x_index++;
      if (pixel_x_index >= npixels_x) {
        pixel_x_index = 0;
        pixel_z_index++;
        if (pixel_z_index >= npixels_z) {
          // Move to next plane
          whichntuple = 1;
          pixel_x_index = 0;
          pixel_y_index = 0;
          pixel_z_index = 0;
          RMGLog::Out(RMGLog::debug, "Moving to YZ plane at event # ", ID + 1);
        }
      }
    } else if (whichntuple == 1) { // YZ plane
      pixel_y_index++;
      if (pixel_y_index >= npixels_y) {
        pixel_y_index = 0;
        pixel_z_index++;
        if (pixel_z_index >= npixels_z) {
          // Move to next plane
          whichntuple = 2;
          pixel_x_index = 0;
          pixel_y_index = 0;
          pixel_z_index = 0;
          RMGLog::Out(RMGLog::debug, "Moving to XY plane at event # ", ID + 1);
        }
      }
    } else if (whichntuple == 2) { // XY plane
      pixel_x_index++;
      if (pixel_x_index >= npixels_x) {
        pixel_x_index = 0;
        pixel_y_index++;
        if (pixel_y_index >= npixels_y) {
          // All pixels complete - shouldn't happen during normal operation
          // The run should end before this, but reset just in case
          RMGLog::Out(RMGLog::warning, "All pixels sampled, resetting to prevent overflow");
          pixel_x_index = 0;
          pixel_y_index = 0;
        }
      }
    }

    // Update current position based on pixel indices
    current_position.setX(limit.x() + pixel_x_index * increment.x());
    current_position.setY(limit.y() + pixel_y_index * increment.y());
    current_position.setZ(limit.z() + pixel_z_index * increment.z());
  }

  // Increment event counter within current pixel
  // But not if we just completed a pixel (to avoid counting the first event of next pixel)
  if (!pixel_just_completed) { current_event_in_pixel++; }

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

  switch (whichntuple) {
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
      RMGLog::Out(RMGLog::fatal, "Invalid whichntuple (", whichntuple, ") in GeneratePrimaries()");
    }
  } // switch(whichntuple)

  fGun->SetParticlePosition({xtemp, ytemp, ztemp});
  fGun->SetParticleMomentumDirection(momentumdir);
  fGun->SetParticleEnergy(1 * u::GeV);

  RMGLog::Out(RMGLog::debug, "Sampled X: ", xtemp, "  Sampled Y: ", ytemp, "  Sampled Z:", ztemp);
  RMGLog::Out(RMGLog::debug, "Momentum direction: ", momentumdir);

  fGun->GeneratePrimaryVertex(event);

} // GeneratePrimaries

// vim: tabstop=2 shiftwidth=2 expandtab

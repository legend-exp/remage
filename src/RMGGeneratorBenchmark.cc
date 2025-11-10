
#include "RMGGeneratorBenchmark.hh"

#include <cmath>
#include <ctime>
#include <algorithm>

#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleMomentum.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "G4AnalysisManager.hh"
#include "Randomize.hh"
#include "G4TransportationManager.hh"
#include "G4TransportationManager.hh"
#include "G4VisExtent.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGNavigationTools.hh"
#include "RMGBenchmarkOutputScheme.hh"
#include "RMGHardware.hh"

namespace u = CLHEP;

RMGGeneratorBenchmark::RMGGeneratorBenchmark() : RMGVGenerator("Benchmark") {
  this->DefineCommands();
  fGun = std::make_unique<G4ParticleGun>();
  
  // Set default values for configurable parameters
  user_increment_x = -1.0;  // negative means auto-calculate (30 pixels default)
  user_increment_y = -1.0;
  user_increment_z = -1.0;
  sampling_width_x = -1.0;  // negative means auto-calculate from world
  sampling_width_y = -1.0;
  sampling_width_z = -1.0;
}


RMGGeneratorBenchmark::~RMGGeneratorBenchmark() = default;


// Helper to find the benchmark output scheme if it's active
RMGBenchmarkOutputScheme* RMGGeneratorBenchmark::GetBenchmarkOutputScheme() {
  auto det_cons = RMGManager::Instance()->GetDetectorConstruction();
  for (auto& scheme : det_cons->GetAllActiveOutputSchemes()) {
    auto benchmark_scheme = dynamic_cast<RMGBenchmarkOutputScheme*>(scheme.get());
    if (benchmark_scheme) return benchmark_scheme;
  }
  return nullptr;
}


void RMGGeneratorBenchmark::PT(G4VPhysicalVolume* volume){

  if(!volume->GetMotherLogical()) return;
  
  for(auto m : RMGNavigationTools::FindDirectMothers(volume))
    {
      trs+= m->GetTranslation();
      //G4cout << m->GetName() << G4endl;
      //G4cout << m->GetTranslation() << G4endl;
      //G4cout << trs << G4endl << G4endl;
      RMGGeneratorBenchmark::PT(m);
    }
  return;//Safety  
}//PT


void RMGGeneratorBenchmark::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Benchmark/",
      "Commands for controlling the benchmarking simulation");

  fMessenger->DeclarePropertyWithUnit("IncrementX", "mm", user_increment_x)
      .SetGuidance("Step size (increment) in X direction (negative = auto, default 30 pixels)")
      .SetParameterName("dx", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("IncrementY", "mm", user_increment_y)
      .SetGuidance("Step size (increment) in Y direction (negative = auto, default 30 pixels)")
      .SetParameterName("dy", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("IncrementZ", "mm", user_increment_z)
      .SetGuidance("Step size (increment) in Z direction (negative = auto, default 30 pixels)")
      .SetParameterName("dz", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("SamplingWidthX", "mm", sampling_width_x)
      .SetGuidance("Sampling width in X direction (negative = auto from world)")
      .SetParameterName("wx", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("SamplingWidthY", "mm", sampling_width_y)
      .SetGuidance("Sampling width in Y direction (negative = auto from world)")
      .SetParameterName("wy", false)
      .SetDefaultValue("-1.0");

  fMessenger->DeclarePropertyWithUnit("SamplingWidthZ", "mm", sampling_width_z)
      .SetGuidance("Sampling width in Z direction (negative = auto from world)")
      .SetParameterName("wz", false)
      .SetDefaultValue("-1.0");
}


void RMGGeneratorBenchmark::BeginOfRunAction(const G4Run* r) {

  //Things set up at the begin of run action:
  // - Start time of entire run (as usual)
  // - Number of events to be processed
  // - The NTuples for storing positional and timing information
  // - The positional limits for the generator
  // - Making sure the user doesn't break the program

  //In this current implementation, these parameters are hardcoded.
  //Need to have a meeting with someone who understands the Python
  //wrapper in order to get these into the CLI.

  //All lengths in mm

  // Initialize default geometry sampling parameters early to avoid
  // using uninitialized member variables (was causing undefined behaviour).
  cubesize = 10000.;
  npixelsperrow = 30;  // Legacy variable, kept for backward compatibility

  xorigin = 0.;
  yorigin = 0.;
  zorigin = 0.;

  xlimit = xorigin - cubesize / 2.;
  ylimit = yorigin - cubesize / 2.;
  zlimit = zorigin - cubesize / 2.;

  G4ThreeVector minn, maxx;//Only used for bounding conditions

  //Specify volume included in the benchmarking simulation (optional)
  std::string targetvolumename = "";

  // NOTE: The benchmark output scheme is registered as an optional output scheme
  // in RMGUserInit::RegisterDefaultOptionalOutputSchemes() and must be activated
  // by the user with: /RMG/Output/ActivateOutputScheme Benchmark
  // It will be initialized by RMGRunAction::SetupAnalysisManager() if active.

  // whichntuple will be initialized to 0 on event 0
  //Start the timer
  starttime   = double(clock())/1000000.;
  currenttime = double(clock())/1000000.;
  
  //Store total number of events to be processed
  totalnevents = r->GetNumberOfEventToBeProcessed();

  //The benchmarker assumes the world volume is a cube when calculating bounds
  auto worldsolid = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume()->GetLogicalVolume()->GetSolid();
  auto typeofworld = worldsolid->GetEntityType();
  if(typeofworld != "G4Box")
    RMGLog::Out(RMGLog::warning, "World entity shape (",typeofworld,") is not a G4Box (why not? :| ). A very large benchmarking cube may crash the simulation.");

  // set the origin and limit according to the world volume
  G4ThreeVector min, max;
  worldsolid->BoundingLimits(min, max);
  
  // Set up sampling dimensions based on user configuration or world bounds
  if (sampling_width_x < 0) {
    sampling_width_x = (max.x() - min.x()) * 1.25;
  }
  if (sampling_width_y < 0) {
    sampling_width_y = (max.y() - min.y()) * 1.25;
  }
  if (sampling_width_z < 0) {
    sampling_width_z = (max.z() - min.z()) * 1.25;
  }
  
  // Calculate increments and number of pixels
  // If user specified increment, use it; otherwise default to 30 pixels
  if (user_increment_x > 0) {
    increment_x = user_increment_x;
    npixels_x = static_cast<int>(std::ceil(sampling_width_x / increment_x));
  } else {
    npixels_x = 30;  // default
    increment_x = sampling_width_x / npixels_x;
  }
  
  if (user_increment_y > 0) {
    increment_y = user_increment_y;
    npixels_y = static_cast<int>(std::ceil(sampling_width_y / increment_y));
  } else {
    npixels_y = 30;  // default
    increment_y = sampling_width_y / npixels_y;
  }
  
  if (user_increment_z > 0) {
    increment_z = user_increment_z;
    npixels_z = static_cast<int>(std::ceil(sampling_width_z / increment_z));
  } else {
    npixels_z = 30;  // default
    increment_z = sampling_width_z / npixels_z;
  }
  
  // Center the sampling region on the world volume center
  xorigin = (max.x() + min.x()) / 2.;
  yorigin = (max.y() + min.y()) / 2.;
  zorigin = (max.z() + min.z()) / 2.;
  
  xlimit = xorigin - sampling_width_x / 2.;
  ylimit = yorigin - sampling_width_y / 2.;
  zlimit = zorigin - sampling_width_z / 2.;
  
  RMGLog::Out(RMGLog::summary, "Benchmark sampling configuration:");
  RMGLog::Out(RMGLog::summary, "  X: ", npixels_x, " pixels, width = ", sampling_width_x, " mm, increment = ", increment_x, " mm");
  RMGLog::Out(RMGLog::summary, "  Y: ", npixels_y, " pixels, width = ", sampling_width_y, " mm, increment = ", increment_y, " mm");
  RMGLog::Out(RMGLog::summary, "  Z: ", npixels_z, " pixels, width = ", sampling_width_z, " mm, increment = ", increment_z, " mm");
  
  //Calculate the number of pixels to simulate, and number of primaries per pixel
  // NOW that we know npixels_x, npixels_y, npixels_z
  
  // Total pixels across all three planes (XZ, YZ, XY)
  int pixels_xz = npixels_x * npixels_z;
  int pixels_yz = npixels_y * npixels_z;
  int pixels_xy = npixels_x * npixels_y;
  totalnpixels = pixels_xz + pixels_yz + pixels_xy;
  
  neventsperpixel = totalnevents / totalnpixels;

  // Calculate events per bunch (1% of events per pixel)
  events_per_bunch = std::max(1, static_cast<int>(std::ceil(neventsperpixel * 0.01)));
  
  RMGLog::Out(RMGLog::summary, "Total pixels to sample: ", totalnpixels, 
              " (XZ: ", pixels_xz, ", YZ: ", pixels_yz, ", XY: ", pixels_xy, ")");
  RMGLog::Out(RMGLog::debug, "Events per 1% bunch: ", events_per_bunch);

  //Safeties
    
  //Could maybe downgrade this from a warning to a summary...
  if(neventsperpixel !=floor(neventsperpixel)){
    RMGLog::Out(RMGLog::warning, "Specified number of primaries(",totalnevents,") doesn't divide evenly into the specified number of pixels (", totalnpixels,")");
    RMGLog::Out(RMGLog::warning, "Rounding down to nearest integer number of primaries per pixel.");
  }  
  neventsperpixel = floor(neventsperpixel);

  //Having a single event sampled per pixel breaks the generator algorithm
  //Doing this would also be completely useless from a benchmarking perspective
  if(neventsperpixel < 2)
    RMGLog::Out(RMGLog::fatal, "Not enough primaries to sample each pixel at least twice! Exiting...");
  else
    RMGLog::Out(RMGLog::summary, "Number of primaries per pixel: ", neventsperpixel);
  
}//BeginOfRunAction


void RMGGeneratorBenchmark::EndOfRunAction(const G4Run* /*r*/) {
  // Save the last pixel if there's any data
  if (!bunch_times.empty() || current_event_in_pixel > 0) {
    // Save any remaining partial bunch
    if (current_event_in_pixel % events_per_bunch != 0 && current_event_in_pixel > 0) {
      double bunch_time = double(clock())/1000000. - bunchstarttime;
      bunch_times.push_back(bunch_time);
    }
    
    RMGLog::Out(RMGLog::debug, "Saving final pixel data in EndOfRunAction");
    SavePixel();
  }
}//EndOfRunAction


void RMGGeneratorBenchmark::SavePixel()
{
  // Find the benchmark output scheme if it's active
  auto benchmark_scheme = GetBenchmarkOutputScheme();
  if (!benchmark_scheme) {
    RMGLog::OutDev(RMGLog::warning, 
        "Benchmark output scheme not active. Use /RMG/Output/ActivateOutputScheme Benchmark");
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
      median_time = (sorted_times[n/2 - 1] + sorted_times[n/2]) / 2.0;
    } else {
      median_time = sorted_times[n/2];
    }
    
    RMGLog::Out(RMGLog::debug, "Pixel complete with ", n, " bunches. Median time: ", median_time, " s");
  }
  RMGLog::Out(RMGLog::debug, "Saving pixel data: Ntuple ", whichntuple,
              ", X: ", xcurrent, ", Y: ", ycurrent, ", Z: ", zcurrent,
              ", Median time: ", median_time, " s");
  benchmark_scheme->SavePixel(whichntuple, xcurrent, ycurrent, zcurrent, median_time);
  
  // Reset for next pixel
  bunch_times.clear();
  current_event_in_pixel = 0;
}//SavePixel


void RMGGeneratorBenchmark::GeneratePrimaries(G4Event* event) {

  ID = event->GetEventID();

  if(ID >= neventsperpixel * totalnpixels)
    return;//Should only apply in situations where the nevents doesn't divide evenly into the npixels
  
  if(ID==0) {
    //Originally initialized in BeginOfRunAction, but delay between that and evt 0 is non-trivial
    currenttime = double(clock())/1000000.;
    bunchstarttime = currenttime;
    current_event_in_pixel = 0;
    bunch_times.clear();
    
    // Initialize pixel indices and position
    whichntuple = 0;
    pixel_x_index = 0;
    pixel_y_index = 0;
    pixel_z_index = 0;
    // Initialize positions centered in the first pixel (consistent with later updates)
    xcurrent = xlimit + 0.5 * increment_x;
    ycurrent = ylimit + 0.5 * increment_y;
    zcurrent = zlimit + 0.5 * increment_z;
  }
  
  // Check if we've completed a 5% bunch
  if (current_event_in_pixel > 0 && current_event_in_pixel % events_per_bunch == 0) {
    double bunch_time = double(clock())/1000000. - bunchstarttime;
    bunch_times.push_back(bunch_time);
    bunchstarttime = double(clock())/1000000.;
    RMGLog::Out(RMGLog::debug, "Bunch complete at event # ", ID, ", time: ", bunch_time, " s");
  }
  
  // Debug: Check pixel completion condition
  int check_value = (ID+1) % int(neventsperpixel);
  if (ID < 100 || check_value == 0) {  // Log first 100 events or when condition is true
    RMGLog::Out(RMGLog::debug, "Event ", ID, ": (ID+1) % neventsperpixel = (", ID+1, ") % ", int(neventsperpixel), " = ", check_value);
  }
  
  bool pixel_just_completed = false;
  if((ID+1) % int(neventsperpixel) == 0) {
    //We'll be changing pixels, so save current data
    // Save the last partial bunch if any events remain
    if (current_event_in_pixel % events_per_bunch != 0) {
      double bunch_time = double(clock())/1000000. - bunchstarttime;
      bunch_times.push_back(bunch_time);
    }
    
    RMGLog::Out(RMGLog::debug, "Current pixel complete at event # ", ID);
    SavePixel();
    pixel_just_completed = true;
    
    // Reset for next pixel and advance to next pixel
    bunchstarttime = double(clock())/1000000.;
    
    // Advance to the next pixel based on current plane
    if (whichntuple == 0) {  // XZ plane
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
    } else if (whichntuple == 1) {  // YZ plane
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
    } else if (whichntuple == 2) {  // XY plane
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
    xcurrent = xlimit + (pixel_x_index + 0.5) * increment_x;
    ycurrent = ylimit + (pixel_y_index + 0.5) * increment_y;
    zcurrent = zlimit + (pixel_z_index + 0.5) * increment_z;
  }
  
  // Increment event counter within current pixel
  // But not if we just completed a pixel (to avoid counting the first event of next pixel)
  if (!pixel_just_completed) {
    current_event_in_pixel++;
  }
       
  fGun->SetNumberOfParticles(1);
  fGun->SetParticleDefinition(G4Geantino::Definition());

  //Randomize particle position within the current grid space
  //For each face of the sampling cube, we will have two
  //randomized positional values, with the current position
  //as a lower bound, and one fixed positional value, which
  //is the face we're currently sampling from
  //To make the code prettier, I'll just sample each
  //value uniformly, then squish whichever one is constant
  
  double xtemp = G4UniformRand()*increment_x+xcurrent;
  double ytemp = G4UniformRand()*increment_y+ycurrent;
  double ztemp = G4UniformRand()*increment_z+zcurrent;

  G4ThreeVector momentumdir(0.,0.,0.);

  switch(whichntuple){
  case 0:{//XZ
    ytemp = ylimit;
    momentumdir.setY(1.);
    break;
  }
  case 1:{//YZ
    xtemp = xlimit;
    momentumdir.setX(1.);
    break;
  }
  case 2:{//XY
    ztemp = zlimit;
    momentumdir.setZ(1.);
    break;
  }
  }//switch(whichntuple)
  
  fGun->SetParticlePosition({xtemp, ytemp, ztemp});
  fGun->SetParticleMomentumDirection(momentumdir);

  const auto& p_tot = 1 * u::GeV;
  const auto& mass = G4Geantino::Definition()->GetPDGMass();
  fGun->SetParticleEnergy(std::sqrt(p_tot * p_tot + mass * mass) - mass);

  RMGLog::Out(RMGLog::debug, "Sampled X: ", xtemp, "  Sampled Y: ", ytemp, "  Sampled Z:", ztemp);
  RMGLog::Out(RMGLog::debug, "Momentum direction: ", momentumdir);
  
  fGun->GeneratePrimaryVertex(event);  
  
}//GeneratePrimaries

// vim: tabstop=2 shiftwidth=2 expandtab

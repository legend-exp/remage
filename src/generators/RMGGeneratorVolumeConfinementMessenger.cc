#include "RMGGeneratorVolumeConfinementMessenger.hh"

#include "globals.hh"
#include "G4PhysicalVolumeStore.hh"

#include "RMGGeneratorVolumeConfinement.hh"
#include "RMGTools.hh"
#include "RMGLog.hh"

RMGGeneratorVolumeConfinementMessenger::RMGGeneratorVolumeConfinementMessenger(RMGGeneratorVolumeConfinement* generator):
  fSampler(generator) {

  G4String directory = "/RMG/Generators/Confinement";

  // mkdir directories
  fDirectories.emplace_back(new G4UIdirectory(directory));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Physical").c_str()));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Geometrical").c_str()));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Geometrical/Sphere").c_str()));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Geometrical/Cylinder").c_str()));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Geometrical/CylindricalShell").c_str()));
  fDirectories.emplace_back(new G4UIdirectory((directory + "/Geometrical/Box").c_str()));

  fSamplingModeCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(
      directory + "/SetSamplingMode", this, "Union IntersectPhysicalWithGeometrical");

  fBoundingSolidTypeCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(
      directory + "/SetFallbackBoundingVolumeType", this, "Sphere Box");

  fAddPhysVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(
      directory + "/Physical/AddVolume", this, "");

  fAddGeomVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAString>(
      directory + "/Geometrical/AddSolid", this, "Sphere Cylinder CylindricalShell Box");

  // Sphere
  fSphereInnerRadiusVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Sphere/InnerRadius", this, "Length", "L", "L >= 0");

  fSphereOuterRadiusVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Sphere/OuterRadius", this, "Length", "L", "L >= 0");

  // Cylinder
  fCylinderInnerRadiusVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Cylinder/InnerRadius", this, "Length", "L", "L >= 0");

  fCylinderOuterRadiusVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Cylinder/OuterRadius", this, "Length", "L", "L >= 0");

  fCylinderHeightVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Cylinder/Height", this, "Length", "L", "L >= 0");

  fCylinderStartingAngleVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Cylinder/StartingAngle", this, "Angle");

  fCylinderSpanningAngleVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Cylinder/SpanningAngle", this, "Angle");

  // Box
  fBoxXLengthVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Box/XLength", this, "Length", "L", "L >= 0");

  fBoxYLengthVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Box/YLength", this, "Length", "L", "L >= 0");

  fBoxZLengthVolCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithADoubleAndUnit>(
      directory + "/Geometrical/Box/ZLength", this, "Length", "L", "L >= 0");

  fGeomVolCenterCmd = RMGTools::MakeG4UIcmd<G4UIcmdWith3VectorAndUnit>(
      directory + "/Geometrical/CenterPosition", this, "Length");

  fNPositionsamplingMaxCmd = RMGTools::MakeG4UIcmd<G4UIcmdWithAnInteger>(
      directory + "/MaxSamplingTrials", this, "N", "N > 0");
}

void RMGGeneratorVolumeConfinementMessenger::SetNewValue(G4UIcommand* cmd, G4String new_values) {

  // little helper
  auto get_last_geom_solid = [&]() -> RMGGeneratorVolumeConfinement::GenericGeometricalSolidData& {
    if (fSampler->GetGeometricalSolidDataList().empty()) {
      RMGLog::Out(RMGLog::fatal, "Must call '", fAddGeomVolCmd->GetTitle(),
          "' before setting any geometrical parameter value");
    }
    return fSampler->GetGeometricalSolidDataList().back();
  };

  if (cmd == fBoundingSolidTypeCmd.get()) {
    fSampler->SetBoundingSolidType(new_values);
  }
  if (cmd == fSamplingModeCmd.get()) {
    if (new_values == "UnionAll") fSampler->SetSamplingMode(RMGGeneratorVolumeConfinement::SamplingMode::kUnionAll);
    else if (new_values == "IntersectionWithGeometrical") fSampler->SetSamplingMode(RMGGeneratorVolumeConfinement::SamplingMode::kIntersectPhysicalWithGeometrical);
  }
  if (cmd == fAddPhysVolCmd.get()) {
    if (new_values.find(' ') == std::string::npos) fSampler->AddPhysicalVolumeNameRegex(new_values);
    else {
      auto name = new_values.substr(0, new_values.find_first_of(' '));
      auto copy_nr = new_values.substr(new_values.find_first_of(' ')+1, std::string::npos);
      fSampler->AddPhysicalVolumeNameRegex(name, copy_nr);
    }
  }
  if (cmd == fAddGeomVolCmd.get()) {
    fSampler->GetGeometricalSolidDataList().emplace_back();
  }
  if (cmd == fSphereInnerRadiusVolCmd.get()) {
    get_last_geom_solid().sphere_inner_radius = fSphereInnerRadiusVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fSphereOuterRadiusVolCmd.get()) {
    get_last_geom_solid().sphere_inner_radius = fSphereOuterRadiusVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fCylinderInnerRadiusVolCmd.get()) {
    get_last_geom_solid().cylinder_inner_radius = fCylinderInnerRadiusVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fCylinderOuterRadiusVolCmd.get()) {
    get_last_geom_solid().cylinder_outer_radius = fCylinderOuterRadiusVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fCylinderHeightVolCmd.get()) {
    get_last_geom_solid().cylinder_height = fCylinderHeightVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fCylinderStartingAngleVolCmd.get()) {
    get_last_geom_solid().cylinder_starting_angle = fCylinderStartingAngleVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fCylinderSpanningAngleVolCmd.get()) {
    get_last_geom_solid().cylinder_spanning_angle = fCylinderSpanningAngleVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fBoxXLengthVolCmd.get()) {
    get_last_geom_solid().box_x_length = fBoxXLengthVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fBoxYLengthVolCmd.get()) {
    get_last_geom_solid().box_y_length = fBoxYLengthVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fBoxZLengthVolCmd.get()) {
    get_last_geom_solid().box_z_length = fBoxZLengthVolCmd->GetNewDoubleValue(new_values);
  }
  if (cmd == fGeomVolCenterCmd.get()) {
    get_last_geom_solid().volume_center = fGeomVolCenterCmd->GetNew3VectorValue(new_values);
  }
  if (cmd == fNPositionsamplingMaxCmd.get()) {
    fSampler->SetMaxAttempts(fNPositionsamplingMaxCmd->GetNewIntValue(new_values));
  }
  else RMGLog::Out(RMGLog::error, "Command ", cmd->GetTitle(), "not known");
}

// vim: tabstop=2 shiftwidth=2 expandtab

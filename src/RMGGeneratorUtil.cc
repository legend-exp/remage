#include "RMGGeneratorUtil.hh"

#include "Randomize.hh"

#include "RMGLog.hh"

#ifndef _g4rand
#define _g4rand() ::G4UniformRand()
#endif

bool RMGGeneratorUtil::IsSampleable(std::string g4_solid_type) {
  if (g4_solid_type == "G4Box" or g4_solid_type == "G4Orb" or g4_solid_type == "G4Sphere" or
      g4_solid_type == "G4Tubs")
    return true;
  else return false;
}

G4ThreeVector RMGGeneratorUtil::rand(const G4VSolid* vol, bool on_surface) {
  if (!vol) RMGLog::OutDev(RMGLog::fatal, "Input G4VSolid* is nullptr");
  auto entity = vol->GetEntityType();
  if (entity == "G4Sphere")
    return RMGGeneratorUtil::rand(dynamic_cast<const G4Sphere*>(vol), on_surface);
  if (entity == "G4Orb") return RMGGeneratorUtil::rand(dynamic_cast<const G4Orb*>(vol), on_surface);
  if (entity == "G4Box") return RMGGeneratorUtil::rand(dynamic_cast<const G4Box*>(vol), on_surface);
  if (entity == "G4Tubs")
    return RMGGeneratorUtil::rand(dynamic_cast<const G4Tubs*>(vol), on_surface);
  else {
    RMGLog::OutDev(RMGLog::fatal, "'", entity, "' is not supported (implement me)");
    return G4ThreeVector();
  }
}

G4ThreeVector RMGGeneratorUtil::rand(const G4Box* box, bool on_surface) {

  if (!box) RMGLog::OutDev(RMGLog::fatal, "Input solid is nullptr");

  auto dx = box->GetXHalfLength();
  auto dy = box->GetYHalfLength();
  auto dz = box->GetZHalfLength();

  if (on_surface) {
    auto A1 = 4 * dx * dy, A2 = 4 * dx * dz, A3 = 4 * dy * dz;
    auto face = _g4rand() * (A1 + A2 + A3);
    auto face_sign = _g4rand() <= 0.5 ? -1 : 1;
    double x, y, z;

    if (face <= A1) {
      x = dx * (2 * _g4rand() - 1);
      y = dy * (2 * _g4rand() - 1);
      z = dz * face_sign;
    } else if (face > A1 and face <= A2) {
      x = dx * (2 * _g4rand() - 1);
      y = dy * face_sign;
      z = dz * (2 * _g4rand() - 1);
    } else {
      x = dx * face_sign;
      y = dy * (2 * _g4rand() - 1);
      z = dz * (2 * _g4rand() - 1);
    }
    return G4ThreeVector(x, y, z);
  } else {
    return G4ThreeVector(dx * (2 * _g4rand() - 1), dy * (2 * _g4rand() - 1),
        dz * (2 * _g4rand() - 1));
  }
}

G4ThreeVector RMGGeneratorUtil::rand(const G4Sphere* sphere, bool on_surface) {

  if (!sphere) RMGLog::OutDev(RMGLog::fatal, "Input solid is nullptr");

  auto r1 = sphere->GetInnerRadius();
  auto r2 = sphere->GetOuterRadius();
  auto phi1 = sphere->GetStartPhiAngle();
  auto delta_phi = sphere->GetDeltaPhiAngle();
  auto cos_theta1 = sphere->GetCosStartTheta();
  auto delta_cos_theta = std::abs(sphere->GetCosEndTheta() - cos_theta1);

  auto phi = phi1 + delta_phi * _g4rand();
  auto cos_theta = delta_cos_theta * _g4rand() + cos_theta1;
  auto s2_point =
      G4ThreeVector(std::cos(phi), std::sin(phi), cos_theta) / std::sqrt(1 + cos_theta * cos_theta);

  if (on_surface) {
    auto A1 = delta_cos_theta * delta_phi * r1 * r1;
    auto A2 = delta_cos_theta * delta_phi * r2 * r2;
    auto side = _g4rand() * (A1 + A2);
    if (side <= A1) return s2_point * r1;
    else return s2_point * r2;
  } else {
    auto R = _g4rand() * (r2 - r1) + r1;
    return s2_point * R;
  }
}

G4ThreeVector RMGGeneratorUtil::rand(const G4Orb* orb, bool on_surface) {
  if (!orb) RMGLog::OutDev(RMGLog::fatal, "Input solid is nullptr");
  return RMGGeneratorUtil::rand(new G4Sphere(orb->GetName(), 0, orb->GetRadius(), 0, CLHEP::twopi,
                                    0, CLHEP::pi),
      on_surface);
}

G4ThreeVector RMGGeneratorUtil::rand(const G4Tubs* tub, bool on_surface) {

  if (!tub) RMGLog::OutDev(RMGLog::fatal, "Input solid is nullptr");

  auto r1 = tub->GetInnerRadius();
  auto r2 = tub->GetOuterRadius();
  auto h = 2 * tub->GetZHalfLength();
  auto a = tub->GetStartPhiAngle();
  auto delta_a = tub->GetDeltaPhiAngle();

  auto phi = a + delta_a * _g4rand();
  auto z = h * (_g4rand() - 0.5);
  auto s1_point = G4ThreeVector(std::cos(phi), std::sin(phi), 0);

  if (on_surface) {
    auto A1 = delta_a * r1 * h;
    auto A2 = delta_a * r2 * h;
    auto A3 = delta_a * (r2 * r2 - r1 * r1);
    auto face = _g4rand() * (A1 + A2 + A3 * 2);
    if (face <= A1) return s1_point * r1 + G4ThreeVector(0, 0, z);
    else if (face > A1 and face <= A2) return s1_point * r2 + G4ThreeVector(0, 0, z);
    else {
      auto face_sign = _g4rand() <= 0.5 ? 1 : -1;
      auto R = _g4rand() * (r2 - r1) + r1;
      return s1_point * R + G4ThreeVector(0, 0, face_sign * h / 2);
    }
  } else {
    auto R = _g4rand() * (r2 - r1) + r1;
    return s1_point * R + G4ThreeVector(0, 0, z);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

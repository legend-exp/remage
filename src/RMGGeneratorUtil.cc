#include <algorithm>
#include <array>
#include <numeric>

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
    std::array<double, 3> A = {4 * dx * dy, 4 * dx * dz, 4 * dy * dz};

    auto face = _g4rand() * std::accumulate(A.begin(), A.end(), 0);
    ;
    auto face_sign = _g4rand() <= 0.5 ? -1 : 1;
    double x, y, z;

    if (face <= A[0]) {
      x = dx * (2 * _g4rand() - 1);
      y = dy * (2 * _g4rand() - 1);
      z = dz * face_sign;
    } else if (face > A[0] and face <= A[0] + A[1]) {
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

  // phi is the equatorial angle [0, 2*pi]
  // theta is the other one [0, pi]

  auto r1 = sphere->GetInnerRadius();
  auto r2 = sphere->GetOuterRadius();
  auto phi1 = sphere->GetStartPhiAngle();
  auto delta_phi = sphere->GetDeltaPhiAngle();
  auto cos_theta1 = sphere->GetCosStartTheta();
  auto delta_cos_theta = sphere->GetCosEndTheta() - cos_theta1;

  auto phi = phi1 + delta_phi * _g4rand();                   // random phi
  auto cos_theta = delta_cos_theta * _g4rand() + cos_theta1; // random cos(theta)
  auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);     // ...and sin(theta)
  // the sampled 3D point on unit sphere
  auto s2_point = G4ThreeVector(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);

  if (on_surface) {

    if (delta_phi != CLHEP::twopi or sphere->GetDeltaThetaAngle() != CLHEP::pi) {
      RMGLog::OutDev(RMGLog::fatal, "Surface sampling on spherical sectors not implemented yet");
    }

    // choose between inner or outer surfaces
    auto A1 = r1 * r1;
    auto A2 = r2 * r2;
    auto side = _g4rand() * (A1 + A2);
    if (side <= A1) return s2_point * r1;
    else return s2_point * r2;
  } else {
    auto u = _g4rand();
    auto R = std::cbrt(u * r2 * r2 * r2 + (1 - u) * r1 * r1 * r1); // random radius
    return s2_point * R;
  }
}

G4ThreeVector RMGGeneratorUtil::rand(const G4Orb* orb, bool on_surface) {

  if (!orb) RMGLog::OutDev(RMGLog::fatal, "Input solid is nullptr");

  auto r = orb->GetRadius();

  auto phi = CLHEP::twopi * _g4rand();                   // random phi
  auto cos_theta = 2 * _g4rand() - 1;                    // random cos(theta)
  auto sin_theta = std::sqrt(1 - cos_theta * cos_theta); // ...and sin(theta)
  // the sampled 3D point on unit sphere
  auto s2_point = G4ThreeVector(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);

  if (on_surface) return s2_point * r;
  else {
    auto u = _g4rand();
    auto R = std::cbrt(u * r * r * r); // random radius
    return s2_point * R;
  }
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
  auto R = _g4rand() * (r2 - r1) + r1;
  auto s1_point = G4ThreeVector(std::cos(phi), std::sin(phi), 0);

  if (on_surface) {
    // choose between inner, outer, top/bottom, side1/side2 surfaces
    std::array<double, 4> A = {
        // areas
        delta_a * r1 * h,                                 // inner
        delta_a * r2 * h,                                 // outer
        delta_a * (r2 * r2 - r1 * r1),                    // top/bottom, (twice)
        (delta_a != CLHEP::twopi) ? 2 * (r2 - r1) * h : 0 // sides (twice) (might not be there)
    };
    auto face = _g4rand() * std::accumulate(A.begin(), A.end(), 0);

    if (face <= A[0]) { // inner
      return s1_point * r1 + G4ThreeVector(0, 0, z);
    } else if (face > A[0] and face <= A[0] + A[1]) { // outer
      return s1_point * r2 + G4ThreeVector(0, 0, z);
    } else if (face > A[0] + A[1] and face <= A[0] + A[1] + A[2]) { // top or bottom
      auto face_sign = _g4rand() <= 0.5 ? 1 : -1;
      return s1_point * R + G4ThreeVector(0, 0, face_sign * h / 2);
    } else { // sides
      auto angle = _g4rand() <= 0.5 ? a : a + delta_a;
      return R * G4ThreeVector(std::cos(angle), std::sin(angle)) + G4ThreeVector(0, 0, z);
    }
  } else {
    return s1_point * R + G4ThreeVector(0, 0, z);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

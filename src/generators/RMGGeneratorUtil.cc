#include "RMGGeneratorUtil.hh"

#include "Randomize.hh"

#ifndef _g4rand
#define _g4rand() ::G4UniformRand()
#endif

G4ThreeVector RMGGeneratorUtil::rand(const G4Box& box, G4bool on_surface) {

  auto dx = box.GetXHalfLength();
  auto dy = box.GetYHalfLength();
  auto dz = box.GetZHalfLength();

  if (on_surface) {
    auto A1 = 4*dx*dy, A2 = 4*dx*dz, A3 = 4*dy*dz;
    auto face = _g4rand()*(A1 + A2 + A3);
    auto face_sign = _g4rand() <= 0.5 ? -1 : 1;
    G4double x, y, z;

    if (face <= A1) {
      x = dx*(2*_g4rand()-1); y = dy*(2*_g4rand()-1); z = dz*face_sign;
    }
    else if (face > A1 and face <= A2) {
      x = dx*(2*_g4rand()-1); y = dy*face_sign; z = dz*(2*_g4rand()-1);
    }
    else {
      x = dx*face_sign; y = dy*(2*_g4rand()-1); z = dz*(2*_g4rand()-1);
    }
    return G4ThreeVector(x, y, z);
  }
  else {
    return G4ThreeVector(dx * (2*_g4rand() - 1),
                         dy * (2*_g4rand() - 1),
                         dz * (2*_g4rand() - 1));
  }
}

G4ThreeVector RMGGeneratorUtil::rand(const G4Sphere& sphere, G4bool on_surface) {

  auto r1 = sphere.GetInnerRadius();
  auto r2 = sphere.GetOuterRadius();

  auto phi = CLHEP::twopi * _g4rand();
  auto cos_theta = 2*_g4rand() - 1;
  auto s2_point = G4ThreeVector(std::cos(phi), std::sin(phi), cos_theta) / std::sqrt(1 + cos_theta*cos_theta);

  if (on_surface) {
    auto A1 = 4*CLHEP::pi*r1*r1;
    auto A2 = 4*CLHEP::pi*r2*r2;
    auto side = _g4rand()*(A1+A2);
    if (side <= A1) return s2_point * r1;
    else return s2_point * r2;
  }
  else {
    auto R = _g4rand()*(r2-r1) + r1;
    return s2_point * R;
  }
}

G4ThreeVector RMGGeneratorUtil::rand(const G4Tubs& tub, G4bool on_surface) {

  auto r1 = tub.GetInnerRadius();
  auto r2 = tub.GetOuterRadius();
  auto h  = 2*tub.GetZHalfLength();

  auto phi = CLHEP::twopi * _g4rand();
  auto z = h * (_g4rand() - 0.5);
  auto s1_point = G4ThreeVector(std::cos(phi), std::sin(phi), 0);

  if (on_surface) {
    auto A1 = CLHEP::twopi*r1*h;
    auto A2 = CLHEP::twopi*r2*h;
    auto A3 = CLHEP::pi*(r2*r2 - r1*r1);
    auto face = _g4rand()*(A1+A2+A3*2);
    if (face <= A1) return s1_point*r1 + G4ThreeVector(0, 0, z);
    else if (face > A1 and face <= A2) return s1_point*r2 + G4ThreeVector(0, 0, z);
    else {
      auto face_sign = _g4rand() <= 0.5 ? 1 : -1;
      auto R = _g4rand()*(r2-r1) + r1;
      return s1_point*R + G4ThreeVector(0, 0, face_sign * h/2);
    }
  }
  else {
    auto R = _g4rand()*(r2-r1) + r1;
    return s1_point*R + G4ThreeVector(0, 0, z);
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab

#include "MGGeneratorUtil.hh"

#include "Randomize.hh"

#include "CLHEP/Units/SystemOfUnits.h"

G4ThreeVector MGGeneratorUtil::pick_isotropic() {

  G4double phi       = M_PI * G4UniformRand();
  G4double cos_theta = 2*G4UniformRand() - 1;
  G4double theta     = std::acos(cos_theta);
  G4double cos_x     = std::sin(theta)*cos(phi);
  G4double cos_y     = std::sin(theta)*sin(phi);
  G4double cos_z     = cos_theta;

  return G4ThreeVector(cos_x, cos_y, cos_z);
}

G4ThreeVector MGGeneratorUtil::pick_point_in_box(G4double x_lo, G4double x_hi,
                                                 G4double y_lo, G4double y_hi,
                                                 G4double z_lo, G4double z_hi) {
  G4double dx = x_hi - x_lo;
  G4double dy = y_hi - y_lo;
  G4double dz = z_hi - z_lo;

  G4double x = x_lo + dx*G4UniformRand();
  G4double y = y_lo + dy*G4UniformRand();
  G4double z = z_lo + dz*G4UniformRand();

  return G4ThreeVector(x, y, z);
}

G4ThreeVector MGGeneratorUtil::pick_point_on_box(G4double x_lo, G4double x_hi,
                                                 G4double y_lo, G4double y_hi,
                                                 G4double z_lo, G4double z_hi) {

  G4double dx = x_hi - x_lo, dy = y_hi - y_lo, dz = z_hi - z_lo;

  // Calculate the fractional area normal to each direction (omitting
  // unnecessary factors of two)
  G4double areaX = dy*dz, areaY = dz*dx, areaZ = dx*dy;
  G4double areaTot = areaX + areaY + areaZ;
  G4double fracAreaX = areaX/areaTot;
  G4double fracAreaY = areaY/areaTot;
  G4double fracAreaZ = areaZ/areaTot;

  // Currently generates 3 random numbers: the first to pick which face of the
  // box, and the other two to choose the point on the face.
  G4double rndFace = G4UniformRand();
  G4double rndFaceXi = G4UniformRand();
  G4double rndFaceXj = G4UniformRand();

  if (rndFace < fracAreaX/2.0) {
    return G4ThreeVector(x_lo, y_lo + dy*rndFaceXi, z_lo + dz*rndFaceXj);
  } else if (rndFace < fracAreaX) {
    return G4ThreeVector(x_hi, y_lo + dy*rndFaceXi, z_lo + dz*rndFaceXj);
  } else if (rndFace < fracAreaX + fracAreaY/2.0) {
    return G4ThreeVector(x_lo + dx*rndFaceXi, y_lo, z_lo + dz*rndFaceXj);
  } else if (rndFace < fracAreaX + fracAreaY) {
    return G4ThreeVector(x_lo + dx*rndFaceXi, y_hi, z_lo + dz*rndFaceXj);
  } else if (rndFace < 1.0 - fracAreaZ/2.0) {
    return G4ThreeVector(x_lo + dx*rndFaceXi, y_lo + dy*rndFaceXj, z_lo);
  } else {
    return G4ThreeVector(x_lo + dx*rndFaceXi, y_lo + dy*rndFaceXj, z_hi);
  }
}

void MGGeneratorUtil::pick_point_in_circle(G4double Radius, G4double &x, G4double &y) {

  G4double b     = Radius*Radius;
  G4double x_rnd = G4UniformRand();
  G4double r     = sqrt(b*x_rnd);
  G4double phi   = M_PI * G4UniformRand();

  x = r*cos(phi);
  y = r*sin(phi);
}

G4ThreeVector MGGeneratorUtil::pick_point_in_annulus(G4double r1, G4double r2, G4double h) {

  G4double a = r1*r1;
  G4double b = r2*r2 - a;

  G4double x_rnd = G4UniformRand();
  G4double r = sqrt(b*x_rnd + a);

  G4double phi = M_PI * G4UniformRand();

  G4double x = r * cos(phi);
  G4double y = r * sin(phi);

  G4double z = h * (-0.5 + G4UniformRand());

  return G4ThreeVector(x, y, z);
}

G4ThreeVector MGGeneratorUtil::pick_point_in_annulus(G4double r1, G4double r2, G4double h,
                                                     G4double theta0, G4double dtheta) {
  G4double a = r1*r1;
  G4double b = r2*r2 - a;

  G4double x_rnd = G4UniformRand();
  G4double r = sqrt(b*x_rnd + a);

  G4double phi = theta0 + dtheta * G4UniformRand();

  G4double x = r * cos(phi);
  G4double y = r * sin(phi);

  G4double z = h * (-0.5 + G4UniformRand());

  return G4ThreeVector(x, y, z);

}

G4ThreeVector MGGeneratorUtil::pick_point_in_cylinder(G4double R, G4double len) {
  return pick_point_in_annulus(0.0, R, len);
}

G4ThreeVector MGGeneratorUtil::pick_point_on_cylinder(G4double R, G4double len) {

  G4double phi = M_PI * G4UniformRand();
  G4double x = R*cos(phi);
  G4double y = R*sin(phi);

  G4double z = len * (-0.5 + G4UniformRand());

  return G4ThreeVector(x, y, z);
}


G4ThreeVector MGGeneratorUtil::pick_point_in_shell(G4double r1, G4double r2) {

  G4double r1_3 = pow(r1,3);
  G4double r2_3 = pow(r2,3);
  G4double d    = r2_3 - r1_3;

  G4double x_rnd = r1_3 + d * G4UniformRand();
  G4double r = pow(x_rnd, 1./3);

  return r*pick_isotropic();
}

G4ThreeVector MGGeneratorUtil::pick_point_in_sphere(G4double r) {
  return pick_point_in_shell(0.0, r);
}

G4ThreeVector MGGeneratorUtil::pick_point_on_sphere(G4double r) {
  return r*pick_isotropic();
}

G4ThreeVector MGGeneratorUtil::pick_point_on_wholetube(G4double r_in, G4double r_out, G4double height) {

  G4double r;
  G4double phi;
  G4double x, y, z;

  G4double a = r_in * r_in;
  G4double b = r_out * r_out - a;

  // calculate areas
  G4double Atop   = 0.5 * M_PI * (r_out * r_out - r_in * r_in);
  G4double Aouter = M_PI * r_out * height;
  G4double Ainner = M_PI * r_in * height;
  G4double Atotal = 2.0 * Atop + Aouter + Ainner;

  // find area to sample on
  G4double x_rand = Atotal * G4UniformRand();

  if (x_rand < Atop) {
    // sample on top
    G4double y_rand = G4UniformRand();

    r   = sqrt(b * y_rand + a);
    phi = M_PI * G4UniformRand();

    x = r * cos(phi);
    y = r * sin(phi);
    z = 0.5 * height;
  }
  else if (x_rand >= Atop && x_rand < 2.0 * Atop) {
    // sample on bottom
    G4double y_rand = G4UniformRand();

    r   = sqrt(b * y_rand + a);
    phi = M_PI * G4UniformRand();

    x =   r * cos(phi);
    y =   r * sin(phi);
    z = - 0.5 * height;
  }
  else if (x_rand >= 2.0 * Atop && x_rand < (2.0 * Atop + Aouter)) {
    // sample on outer area

    phi = M_PI * G4UniformRand();

    x = r_out * cos(phi);
    y = r_out * sin(phi);
    z = height * (-0.5 + G4UniformRand());
  }
  else {
    // sample on inner area
    phi = M_PI * G4UniformRand();

    x = r_in * cos(phi);
    y = r_in * sin(phi);
    z = height * (-0.5 + G4UniformRand());
  }

  return G4ThreeVector(x, y, z);

}

// vim: tabstop=2 shiftwidth=2 expandtab

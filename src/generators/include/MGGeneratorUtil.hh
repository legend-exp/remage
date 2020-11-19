#ifndef _MGGENERATORUTIL_HH
#define _MGGENERATORUTIL_HH

#include "globals.hh"
#include "G4ThreeVector.hh"

namespace MGGeneratorUtil {

    G4ThreeVector pick_isotropic();

    G4ThreeVector pick_point_in_box(G4double x_lo, G4double x_hi,
                                    G4double y_lo, G4double y_hi,
                                    G4double z_lo, G4double z_hi);

    G4ThreeVector pick_point_on_box(G4double x_lo, G4double x_hi,
                                    G4double y_lo, G4double y_hi,
                                    G4double z_lo, G4double z_hi);

    G4ThreeVector pick_point_in_annulus(G4double r1, G4double r2, G4double h);

    G4ThreeVector pick_point_in_annulus(G4double r1, G4double r2, G4double h,
                                        G4double theta0, G4double dtheta);

    G4ThreeVector pick_point_in_cylinder(G4double Radius, G4double L);

    G4ThreeVector pick_point_on_cylinder(G4double Radius, G4double L);

    G4ThreeVector pick_point_in_shell(G4double r1, G4double r2);

    G4ThreeVector pick_point_in_sphere(G4double r);

    G4ThreeVector pick_point_on_sphere(G4double r);

    G4ThreeVector pick_point_on_wholetube(G4double r_in, G4double r_out, G4double height); 

    void pick_point_in_circle(G4double Radius, G4double &x, G4double &y);
}

#endif

// vim: tabstop=2 shiftwidth=2 expandtab

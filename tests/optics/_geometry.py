from __future__ import annotations

import numpy as np
import pint
import pyg4ometry as pg4

u = pint.get_application_registry()


def _add_dummy_sipm_surface(
    reflectivity: float,
    efficiency: float,
    reg: pg4.geant4.Registry,
    det_l: pg4.geant4.LogicalVolume,
) -> None:
    det_surf = pg4.geant4.solid.OpticalSurface(
        "det_surf",
        finish="ground",
        model="unified",
        surf_type="dielectric_metal",
        value=0.05,
        registry=reg,
    )

    λ = np.array([100, 800]) * u.nm
    eff = np.array([efficiency, efficiency])
    refl = np.array([reflectivity, reflectivity])

    with u.context("sp"):
        det_surf.addVecPropertyPint("EFFICIENCY", λ.to("eV"), eff)
        det_surf.addVecPropertyPint("REFLECTIVITY", λ.to("eV"), refl)

    pg4.geant4.SkinSurface("det_surf", det_l, det_surf, reg)

from __future__ import annotations

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
import pint
import pyg4ometry as pg4
import pygeomoptics
from _geometry import _add_dummy_sipm_surface
from lgdo import lh5
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run
from scipy.optimize import curve_fit

u = pint.get_application_registry()

macro = """
/RMG/Processes/OpticalPhysics
/RMG/Processes/OpticalPhysicsMaxOneWLSPhoton {max_one_ph}

/RMG/Geometry/RegisterDetector Optical detector 001

/RMG/Output/ActivateOutputScheme Track

/run/initialize

/RMG/Output/Track/StoreOpticalPhotons
/RMG/Output/Track/StoreSinglePrecisionEnergy
/RMG/Output/Track/StoreSinglePrecisionPosition

/process/optical/wls/setTimeProfile {time_profile}

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     0 0 0 mm
/gps/particle     opticalphoton
/gps/energy       9.68 eV
/gps/direction    0 0 1

/run/beamOn {events}
"""

chamber_length_mm = 200


def geometry_attenuation(wls_thickness: float):
    reg = pg4.geant4.Registry()
    matreg = LegendMaterialRegistry(reg, enable_optical=False)

    world_s = pg4.geant4.solid.Box("world", 1000, 1000, 1000, registry=reg, lunit="mm")
    world_l = pg4.geant4.LogicalVolume(world_s, "G4_Galactic", "world", registry=reg)
    reg.setWorld(world_l)

    mat = matreg.liquidargon
    pygeomoptics.lar.pyg4_lar_attach_rindex(mat, reg)

    chamber_length_mm = 200
    chamber_s = pg4.geant4.solid.Tubs(
        "chamber", 0, 100, chamber_length_mm, 0, 2 * np.pi, reg, "mm"
    )
    chamber_l = pg4.geant4.LogicalVolume(chamber_s, mat, "chamber", registry=reg)
    pg4.geant4.PhysicalVolume(
        [0, 0, 0], [0, 0, 0], chamber_l, "chamber", world_l, registry=reg
    )

    det_s = pg4.geant4.solid.Sphere(
        "detector", 20, 20.1, 0, 2 * np.pi, 0, np.pi * 0.98, reg, "mm"
    )
    det_l = pg4.geant4.LogicalVolume(
        det_s, matreg.metal_silicon, "detector", registry=reg
    )
    pg4.geant4.PhysicalVolume(
        [0, 0, 0],
        [0, 0, chamber_length_mm / 4, "mm"],
        det_l,
        "detector",
        chamber_l,
        registry=reg,
    )

    _add_dummy_sipm_surface(0, 1, reg, det_l)

    mat = matreg.tpb_on_fibers
    # note: attach the rindex of _LAr_ here, to avoid refraction/reflection at the boundary.
    pygeomoptics.lar.pyg4_lar_attach_rindex(mat, reg)

    # attach some dummy WLS properties.
    wls_abs_wvl = np.array([100, 105, 195, 200]) * u.nm
    wls_abs = 2 * u.m / np.array([0.00001, 1000, 1000, 0.00001])

    wls_em_wvl = pygeomoptics.pyg4utils.pyg4_sample_λ(210 * u.nm, 300 * u.nm)
    wls_em = np.ones_like(wls_em_wvl) * wls_em_wvl**2 / wls_em_wvl[0] ** 2
    wls_em[0] = 0
    wls_em[-1] = 0

    with u.context("sp"):
        mat.addVecPropertyPint("WLSABSLENGTH", wls_abs_wvl.to("eV"), wls_abs)
        mat.addVecPropertyPint("WLSCOMPONENT", wls_em_wvl.to("eV"), wls_em)

    mat.addConstPropertyPint("WLSTIMECONSTANT", 1 * u.us)
    mat.addConstPropertyPint("WLSMEANNUMBERPHOTONS", 1)

    wls_s = pg4.geant4.solid.Tubs(
        "wls_disk", 0, 5, wls_thickness, 0, 2 * np.pi, reg, "mm"
    )
    wls_l = pg4.geant4.LogicalVolume(wls_s, mat, "wls_disk", registry=reg)
    pg4.geant4.PhysicalVolume(
        [0, 0, 0],
        [0, 0, chamber_length_mm / 4, "mm"],
        wls_l,
        "wls_disk",
        chamber_l,
        registry=reg,
    )

    return reg


def simulate(
    wls_thickness: float,
    time_profile: str | None = None,
    enable_rmg_op_wls: bool | None = None,
) -> str:
    output_prof = "" if time_profile is None else f"-{time_profile}"
    output_rmgwls = (
        "" if enable_rmg_op_wls is None else f"-rmgopwls-{enable_rmg_op_wls}"
    )
    output = f"output-wls-{wls_thickness:.2f}{output_prof}{output_rmgwls}.lh5"

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "events": 20000,
            "time_profile": time_profile or "delta",
            "max_one_ph": str(
                enable_rmg_op_wls if enable_rmg_op_wls is not None else True
            ).lower(),
        },
        gdml_files=geometry_attenuation(wls_thickness),
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


def test_wls():
    fig, ax = plt.subplots()

    x = np.arange(1, 10)
    sh = np.zeros_like(x)
    tr = np.zeros_like(x)

    spectra = [None] * len(x)

    for i, wls_thickness in enumerate(x):
        remage_output = simulate(wls_thickness)

        stps = lh5.read_as("stp/det001", remage_output, library="ak", with_units=True)
        sh[i] = ak.count(stps.wavelength[stps.wavelength > 200])
        tr[i] = ak.count(stps.wavelength[stps.wavelength < 200])

        tracks = lh5.read_as("tracks", remage_output, library="ak")

        # read in dictionary with process ids, to filter later
        processes = lh5.read("processes", remage_output)
        proc_wls = processes["RMGOpWLS"].value

        tracks_wvl = 1239 / tracks.ekin[tracks.procid == proc_wls] / 1e6
        spectra[i] = hist.new.Reg(100, 0, 400).Double().fill(tracks_wvl)

    def shifted_fit(x, a, b):
        return a * (1 - np.exp(-b * x))

    popt, _pcov = curve_fit(shifted_fit, x, sh, p0=(10000, 0.5))

    x_exp = np.linspace(1, 9)
    ax.plot(x_exp, shifted_fit(x_exp, *popt), linestyle="--")

    display_label = f"$\\Lambda = {1 / popt[1] / 10:.1f}\\mathrm{{ cm}}$"
    ax.scatter(x, sh, marker=".", zorder=1, label=f"> 200 nm ({display_label})")
    ax.scatter(x, tr, marker="x", zorder=1, label="< 200 nm")
    ax.scatter(x, sh + tr, marker="+", zorder=1, label="total")

    ax.legend()
    ax.set_title("shifted photons")
    ax.set_ylabel("number of detected photons")
    ax.set_xlabel("WLS disk thickness [mm]")
    fig.savefig("photon-wls.output.png")

    fig, ax = plt.subplots()
    for wls_thickness, sp in zip(x, spectra, strict=True):
        sp.plot(ax=ax, label=str(wls_thickness), yerr=False)
    ax.legend()
    ax.set_yscale("log")
    fig.savefig("photon-wls-spectrum.output.png")


def test_wls_time():
    for time_profile in ("delta", "exponential"):
        fig, ax = plt.subplots()
        remage_output = simulate(6, time_profile=time_profile)

        stps = lh5.read_as("stp/det001", remage_output, library="ak", with_units=True)
        wls_time_hist = (
            hist.new.Reg(40, 0, 400)
            .Reg(20, 0, 5)
            .Double()
            .fill(ak.flatten(stps.wavelength), ak.flatten(stps.time) / 1e3)
        )
        wls_time_hist.plot(ax=ax)

        ax.set_title("arrival times of (shifted) photons")
        ax.set_ylabel("arrival times [us]")
        ax.set_xlabel("wavelength [nm]")
        fig.savefig(f"photon-wls-times-{time_profile}.output.png")


def test_wls_photons():
    fig, ax = plt.subplots()
    for enable_rmg_op_wls in (True, False):
        remage_output = simulate(6, enable_rmg_op_wls=enable_rmg_op_wls)

        tracks = lh5.read_as("tracks", remage_output, library="ak")

        # filter out tracks from processes
        processes = lh5.read("processes", remage_output)
        proc_name = "RMGOpWLS" if enable_rmg_op_wls else "OpWLS"
        proc_wls = processes[proc_name].value
        tracks = tracks[tracks.procid == proc_wls]

        wls_time_hist = (
            hist.new.Reg(8, 0, 8).Double().fill(ak.run_lengths(tracks.evtid))
        )
        wls_time_hist.plot(ax=ax, label=proc_name, yerr=False)

    ax.legend()
    ax.set_title("number of emitted WLS photons")
    ax.set_ylabel("counts [a.u.]")
    ax.set_xlabel("number of emitted WLS photon")
    fig.savefig("photon-wls-photon-count.output.png")

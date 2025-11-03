from __future__ import annotations

import awkward as ak
import boost_histogram as bh
import hist
import matplotlib.pyplot as plt
from lgdo import lh5

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12
plt.rcParams["figure.figsize"] = (10, 3)

style = {"yerr": False, "flow": None, "fill": True, "lw": 0.6, "alpha": 0.5}


def plot_vertex_energy():
    plt.subplots()
    tracks = lh5.read_as("tracks", "track.lh5", "ak")

    pid = tracks.particle  # PDG codes
    parentid = tracks.parent_trackid  # parent track ID
    evtid = tracks.evtid
    ekin = tracks.ekin

    # Masks for gamma candidates
    mask_gamma = pid == 22

    # Find all Ar39 parents (PDGID = 1000180390)
    mask_ar39 = pid == 1000180390
    parent_evtid = ak.to_numpy(evtid[mask_ar39])
    parent_trkid = ak.to_numpy(tracks.trackid[mask_ar39])

    # Build a set of valid (event, track) pairs
    ar39_pairs = set(zip(parent_evtid, parent_trkid, strict=True))

    # Build mask for gamma whose parent is in that set
    gamma_evtid = ak.to_numpy(evtid[mask_gamma])
    gamma_parent = ak.to_numpy(parentid[mask_gamma])

    is_from_ar39 = [
        (e, p) in ar39_pairs for e, p in zip(gamma_evtid, gamma_parent, strict=True)
    ]

    # Select the gamma energies
    gamma_from_ar39_ekin = ak.to_numpy(ekin[mask_gamma][is_from_ar39])

    h = hist.new.Reg(75, 0, 0.5, name="$E_\\gamma$ [MeV]").Double()
    h.fill(gamma_from_ar39_ekin)

    h.plot(yerr=False)
    plt.title("The Inner Bremsstrahlung Gamma Vertex Energy spectrum")
    plt.xlabel("vertex energy [MeV]")
    plt.ylabel("Counts / bin")
    plt.yscale("log")
    plt.savefig("innerbrem-gamma-vertex-energy.output.png")
    plt.show()


def get_histogram(detid, filename):
    """Read data and return a filled histogram."""
    data = lh5.read_as(f"hit/{detid}", filename, "ak")

    h = hist.new.Reg(100, 0, 500, name="energy [keV]").Double()
    h.fill(data.active_energy)
    return h


def plot_energy_comparison():
    h_IB = get_histogram("det001", "output_IB.lh5")
    h_noIB = get_histogram("det001", "output_noIB.lh5")

    h_IB_rebinned = h_IB.copy()
    h_IB_rebinned = h_IB_rebinned[bh.rebin(2)]

    h_noIB_rebinned = h_noIB.copy()
    h_noIB_rebinned = h_noIB_rebinned[bh.rebin(2)]

    diff_rebinned = h_IB - h_noIB
    diff_rebinned = diff_rebinned[bh.rebin(2)]

    # Plot the rebinned histograms
    plt.subplots(figsize=(10, 3))
    h_IB_rebinned.plot(yerr=False, label="With IB")
    h_noIB_rebinned.plot(yerr=False, label="Without IB")

    plt.ylabel("counts / 10 keV")
    plt.legend()
    plt.savefig("innerbrem-signal.output.png")

    # Plot the rebinned difference
    plt.subplots(figsize=(10, 3))
    diff_rebinned.plot(yerr=False, label="difference")
    plt.axhline(0, color="k", linestyle="--", linewidth=1)
    plt.ylabel("counts difference / 20 keV")
    plt.legend()
    plt.savefig("innerbrem-difference.output.png")


plot_vertex_energy()
plot_energy_comparison()

from __future__ import annotations

import subprocess

import matplotlib.pyplot as plt

plt.rcParams.update(
    {
        "figure.figsize": (10, 6),
        "figure.dpi": 120,
        "font.size": 14,
        "axes.titlesize": 20,
        "axes.labelsize": 16,
        "xtick.labelsize": 13,
        "ytick.labelsize": 13,
        "legend.fontsize": 13,
        "lines.linewidth": 2,
        "axes.grid": True,
        "grid.alpha": 0.3,
    }
)

COLOR_CYCLE = plt.get_cmap("tab10").colors


def style_axes(ax, xlabel, ylabel, title):
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.tick_params(direction="in", which="both", length=6)


def save(fig, name):
    fig.tight_layout()
    fig.savefig(name)
    plt.close(fig)


version = subprocess.run(
    ["geant4-config", "--version"],
    check=False,
    capture_output=True,
    text=True,
)
G4_VERSION_NAME = version.stdout.strip()  # remove any trailing newline


# Takes the pdg code of the isotope after capture
# Returns the isotope label of the isotope that was captured on.
def pdg_to_isotope(pdg: int) -> str:
    pdg = int(pdg)
    if abs(pdg) < 1_000_000_000:
        return f"PDG {pdg}"
    # The minus 1 converts from the Geant4 trackoutputscheme which yields the isotope AFTER capture.
    # But here we are always talking about the isotope the capture happened on.
    A = (pdg // 10) % 1000 - 1
    Z = (pdg // 10000) % 1000

    element = [
        "n",
        "H",
        "He",
        "Li",
        "Be",
        "B",
        "C",
        "N",
        "O",
        "F",
        "Ne",
        "Na",
        "Mg",
        "Al",
        "Si",
        "P",
        "S",
        "Cl",
        "Ar",
        "K",
        "Ca",
        "Sc",
        "Ti",
        "V",
        "Cr",
        "Mn",
        "Fe",
        "Co",
        "Ni",
        "Cu",
        "Zn",
        "Ga",
        "Ge",
        "As",
        "Se",
        "Br",
        "Kr",
        "Rb",
        "Sr",
        "Y",
        "Zr",
        "Nb",
        "Mo",
        "Tc",
        "Ru",
        "Rh",
        "Pd",
        "Ag",
        "Cd",
        "In",
        "Sn",
        "Sb",
        "Te",
        "I",
        "Xe",
        "Cs",
        "Ba",
        "La",
        "Ce",
        "Pr",
        "Nd",
        "Pm",
        "Sm",
        "Eu",
        "Gd",
    ]

    symbol = element[Z] if len(element) > Z else f"Z={Z}"
    return f"{symbol}-{A}"

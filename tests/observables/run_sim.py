from __future__ import annotations

import os
import sys
from multiprocessing import Pool
from pathlib import Path

import dbetto
from reboost.build_hit import build_hit
from remage import remage_run

rmg = sys.argv[1]
n_proc = int(os.environ.get("RMG_STATS_FACTOR", "1"))
n_events = 20000 * n_proc * (4 if n_proc > 1 else 1)


def replace_lines(
    input_file: str, output_file: str, replacements: dict[str, str]
) -> None:
    """Replaces lines in a file that match given patterns.

    Parameters:
    - input_file: Path to the input text file.
    - output_file: Path to save the modified text file.
    - replacements: A dictionary where keys are regex patterns to match lines,
      and values are the replacement strings.
    """
    with Path(input_file).open(encoding="utf-8") as f:
        lines = f.readlines()

    with Path(output_file).open("w", encoding="utf-8") as f:
        for line in lines:
            line_t = line
            for pattern, replacement in replacements.items():
                if pattern in line:
                    line_t = replacement + "\n"
                    break
            f.write(line_t)


def run_reboost(generator_name, name, val, reboost_config="config/hit_config.yaml"):
    path = f"{generator_name}/{name}/max_{val}/"

    # directories
    stp_directory = Path(f"out/{path}/stp/")
    hit_directory = Path(f"out/{path}/hit/")

    # make the directories
    hit_directory.mkdir(parents=True, exist_ok=True)

    stp_files = [f"{stp_directory}/out.lh5"]
    hit_files = [f"{hit_directory}/out.lh5"]

    args = dbetto.AttrsDict({"gdml": "gdml/geometry.gdml"})
    _, _ = build_hit(
        reboost_config,
        args=args,
        stp_files=stp_files,
        glm_files=None,
        hit_files=hit_files,
        buffer=10_000_000,
        overwrite=True,
    )


def run_sim(
    generator_name="",
    name="",
    val="0",
    step_limits="",
    prod_cuts="",
    step_points="",
    proc="",
    generator="",
    register_lar=False,
):
    macro_file = "mac.mac"
    dir_string = f"{generator_name}/{name}/max_{val}/"

    # make the out directory
    stp_directory = Path(f"out/{dir_string}/stp/")
    macro_directory = Path(f"macros/{dir_string}/")

    lar_command = (
        "/RMG/Geometry/RegisterDetector Scintillator LAr 002" if register_lar else ""
    )

    stp_directory.mkdir(parents=True, exist_ok=True)
    macro_directory.mkdir(parents=True, exist_ok=True)

    replacements = {
        "$STEP_LIMITS_COMMAND": step_limits,
        "$PROD_CUTS_COMMAND": prod_cuts,
        "$GENERATOR": generator,
        "$STEP_POINT": step_points,
        "$PROC": proc,
        "$REGISTER_LAR": lar_command,
    }
    replace_lines(
        "macros/template.mac", macro_directory / Path(macro_file), replacements
    )
    remage_run(
        str(macro_directory / macro_file),
        macro_substitutions={"NEVENTS": str(n_events)},
        gdml_files="gdml/geometry.gdml",
        output=f"{stp_directory}/out.lh5",
        overwrite_output=True,
        threads=1,
    )


do_bulk = True
do_surf = True
energy = 1000

generators = {}
cuts = [10, 20, 50, 100, 200, None]


# define some generator commands
if do_surf:
    generators["beta_surf"] = f"""
/RMG/Generator/Select GPS
/gps/position 0 0 -20 mm
/gps/particle e-
/gps/energy {energy} keV
/gps/direction 0 0 1
"""

if do_bulk:
    generators["beta_bulk"] = f"""
/RMG/Generator/Confine Volume
/RMG/Generator/Confinement/Physical/AddVolume germanium
/RMG/Generator/Select GPS
/gps/particle e-
/gps/ang/type iso
/gps/energy {energy} keV
"""


def run_sim_and_pproc(gen):
    generator, config = gen

    # loop over step limits
    for step_limits in cuts:
        command = (
            f"/RMG/Geometry/SetMaxStepSize {step_limits} um germanium"
            if step_limits is not None
            else ""
        )

        # run the simulation
        run_sim(
            generator_name=generator,
            name="step_limits",
            val=step_limits,
            step_limits=command,
            prod_cuts="",
            proc="",
            step_points="/RMG/Output/Germanium/StepPositionMode Both",
            generator=config,
            register_lar=False,
        )

        # post-process it
        run_reboost(
            generator_name=generator,
            name="step_limits",
            val=step_limits,
            reboost_config="config/hit_config.yaml",
        )


if __name__ == "__main__":
    with Pool(n_proc) as pool:
        pool.map(run_sim_and_pproc, list(generators.items()))

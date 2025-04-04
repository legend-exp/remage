from __future__ import annotations

import logging
import subprocess
import sys
from pathlib import Path

import colorlog
import dbetto
from reboost.build_glm import build_glm
from reboost.build_hit import build_hit

log = logging.getLogger(__name__)

handler = colorlog.StreamHandler()
handler.setFormatter(
    colorlog.ColoredFormatter("%(log_color)s%(name)s [%(levelname)s] %(message)s")
)
logger = logging.getLogger()
logger.handlers.clear()
logger.addHandler(handler)
logger.setLevel(logging.INFO)


rmg = sys.argv[1]


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
    glm_directory = Path(f"out/{path}/glm/")
    hit_directory = Path(f"out/{path}/hit/")

    # make the directories
    hit_directory.mkdir(parents=True, exist_ok=True)
    glm_directory.mkdir(parents=True, exist_ok=True)

    glm_files = [f"{glm_directory}/out.lh5"]
    stp_files = [f"{stp_directory}/out.lh5"]
    hit_files = [f"{hit_directory}/out.lh5"]

    build_glm(
        glm_files=glm_files,
        stp_files=stp_files,
        id_name="evtid",
    )

    args = dbetto.AttrsDict({"gdml": "gdml/geometry.gdml"})
    _, _ = build_hit(
        reboost_config,
        args=args,
        stp_files=stp_files,
        glm_files=glm_files,
        hit_files=hit_files,
        buffer=10_000_000,
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
    subprocess.run(
        f"{rmg} {macro_directory / macro_file} -g gdml/geometry.gdml -o {stp_directory}/out.lh5 -w -t 1  ",
        shell=True,
        check=False,
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


# with and without the argon table
profile = {}
for generator, config in generators.items():
    profile[generator] = {}

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

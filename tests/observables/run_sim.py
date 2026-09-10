from __future__ import annotations

import json
import os
import resource
import shutil
import sys
import time
from multiprocessing import Pool
from pathlib import Path

from build_hit import build_hit
from remage import remage_run

rmg = sys.argv[1]
n_proc = int(os.environ.get("RMG_STATS_FACTOR", "1"))
n_events = 20000 * n_proc * (4 if n_proc > 1 else 1)

# the step files are large and only needed to build the hit files, so they are
# deleted as soon as the post-processing is done with them: keeping all of
# them fills up the CI runner's disk. The two listed here are the ones
# plot_steps.py reads (see CMakeLists.txt). Set RMG_KEEP_STP=1 to keep all of
# them around for debugging.
keep_stp = os.environ.get("RMG_KEEP_STP", "") not in ("", "0")
keep_stp_scan_points = {
    ("beta_bulk", "step_limits", None),
    ("beta_bulk", "step_limits", 10),
}


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


def run_pproc(generator_name, name, val):
    path = f"{generator_name}/{name}/max_{val}/"

    # directories
    stp_directory = Path(f"out/{path}/stp/")
    hit_directory = Path(f"out/{path}/hit/")

    # make the directories
    hit_directory.mkdir(parents=True, exist_ok=True)

    build_hit(f"{stp_directory}/out.lh5", f"{hit_directory}/out.lh5")


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

    # the simulation runs in a child process, so its CPU time can be extracted
    # from the resource usage of the children of this worker process. Contrary
    # to the wall-clock time this is insensitive to the machine load, i.e. to
    # how many jobs of this test are running in parallel.
    rusage_start = resource.getrusage(resource.RUSAGE_CHILDREN)
    wall_start = time.perf_counter()

    remage_run(
        str(macro_directory / macro_file),
        macro_substitutions={"NEVENTS": str(n_events)},
        gdml_files="gdml/geometry.gdml",
        output=f"{stp_directory}/out.lh5",
        overwrite_output=True,
        threads=1,
    )

    wall_time = time.perf_counter() - wall_start
    rusage_end = resource.getrusage(resource.RUSAGE_CHILDREN)
    cpu_time = (rusage_end.ru_utime - rusage_start.ru_utime) + (
        rusage_end.ru_stime - rusage_start.ru_stime
    )

    output_size = sum(f.stat().st_size for f in stp_directory.glob("out*.lh5"))

    with (Path(f"out/{dir_string}") / "timing.json").open(
        "w", encoding="utf-8"
    ) as timing_file:
        json.dump(
            {
                "generator": generator_name,
                "name": name,
                "val": val,
                "n_events": n_events,
                "cpu_time": cpu_time,
                "wall_time": wall_time,
                "output_size": output_size,
                "n_parallel_jobs": n_proc,
            },
            timing_file,
            indent=2,
        )


do_bulk = True
do_surf = True
energy = 1000

generators = {}
all_step_limits = [10, 20, 50, 100, 200, None]
all_prod_cuts = [0.01, 0.02, 0.05, 0.3, 0.5, 0.7, 1, None]


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

jobs = []
for generator, config in generators.items():
    for limit in all_step_limits:
        jobs.append((generator, config, limit, None, "step_limits"))
    for cut in all_prod_cuts:
        jobs.append((generator, config, None, cut, "prod_cuts"))


def run_sim_and_pproc(gen):
    generator, config, step_limits, prod_cuts, mode = gen

    step_limits_command = (
        f"/RMG/Geometry/SetMaxStepSize {step_limits} um germanium"
        if step_limits is not None
        else ""
    )
    prod_cuts_command = (
        f"/RMG/Processes/SensitiveProductionCut {prod_cuts} mm"
        if prod_cuts is not None
        else ""
    )

    name_kwargs = {"name": "step_limits", "val": step_limits}
    if mode == "prod_cuts":
        name_kwargs = {"name": "prod_cuts", "val": prod_cuts}

    # run the simulation
    run_sim(
        generator_name=generator,
        step_limits=step_limits_command,
        prod_cuts=prod_cuts_command,
        proc="",
        step_points="/RMG/Output/Germanium/StepPositionMode Both",
        generator=config,
        register_lar=False,
        **name_kwargs,
    )

    # post-process it
    run_pproc(generator_name=generator, **name_kwargs)

    # the output size has already been recorded in timing.json above, so the
    # step file can go unless something downstream still reads it
    scan_point = (generator, name_kwargs["name"], name_kwargs["val"])
    if not keep_stp and scan_point not in keep_stp_scan_points:
        shutil.rmtree(
            Path(
                f"out/{generator}/{name_kwargs['name']}/max_{name_kwargs['val']}/stp/"
            ),
            ignore_errors=True,
        )


if __name__ == "__main__":
    with Pool(n_proc) as pool:
        pool.map(run_sim_and_pproc, jobs)

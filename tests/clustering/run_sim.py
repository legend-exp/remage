from __future__ import annotations

import json
import os
import resource
import shutil
import sys
import time
from multiprocessing import Pool
from pathlib import Path

import dbetto
from reboost.build_hit import build_hit
from remage import remage_run

rmg = sys.argv[1] if len(sys.argv) > 1 else None
n_proc = int(os.environ.get("RMG_STATS_FACTOR", "1"))
n_events = 20000 * n_proc * (2 if n_proc > 1 else 1)

# the step files of this scan are large (a run without pre-clustering writes
# ~2.7 kB/event) and are only needed to build the hit files, so they are
# deleted as soon as reboost is done with them. Set RMG_KEEP_STP=1 to keep
# them around for debugging.
keep_stp = os.environ.get("RMG_KEEP_STP", "") not in ("", "0")

# the germanium pre-clustering settings scanned here. ``None`` always means
# "pre-clustering completely disabled" and is used as the reference point.
all_cluster_distances = [10, 20, 50, 100, 200, 500, 1000, None]  # um
all_cluster_distances_surface = [1, 5, 10, 20, 50, 100, None]  # um
all_cluster_times = [0.1, 10, 1000, None]  # us

energy = 1000  # keV

# the macro snippet configuring the pre-clustering for a given scan point
cluster_commands = {
    "cluster_distance": "/RMG/Output/Germanium/Cluster/PreClusterDistance {} um",
    "cluster_distance_surface": (
        "/RMG/Output/Germanium/Cluster/PreClusterDistanceSurface {} um"
    ),
    "cluster_time": "/RMG/Output/Germanium/Cluster/PreClusterTimeThreshold {} us",
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


def run_sim(generator_name="", name="", val="0", cluster="", generator=""):
    macro_file = "mac.mac"
    dir_string = f"{generator_name}/{name}/max_{val}/"

    # make the out directory
    stp_directory = Path(f"out/{dir_string}/stp/")
    macro_directory = Path(f"macros/{dir_string}/")

    stp_directory.mkdir(parents=True, exist_ok=True)
    macro_directory.mkdir(parents=True, exist_ok=True)

    replacements = {
        "$CLUSTER_COMMANDS": cluster,
        "$GENERATOR": generator,
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


# the same generators as in the observables-ge test: electrons in the bulk of
# the germanium and electrons impinging on its surface
generators = {
    "beta_surf": f"""
/RMG/Generator/Select GPS
/gps/position 0 0 -20 mm
/gps/particle e-
/gps/energy {energy} keV
/gps/direction 0 0 1
""",
    "beta_bulk": f"""
/RMG/Generator/Confine Volume
/RMG/Generator/Confinement/Physical/AddVolume germanium
/RMG/Generator/Select GPS
/gps/particle e-
/gps/ang/type iso
/gps/energy {energy} keV
""",
}

jobs = []
for generator, config in generators.items():
    for name, values in (
        ("cluster_distance", all_cluster_distances),
        ("cluster_distance_surface", all_cluster_distances_surface),
        ("cluster_time", all_cluster_times),
    ):
        for val in values:
            jobs.append((generator, config, name, val))


def run_sim_and_pproc(job):
    generator, config, name, val = job

    # None means: do not pre-cluster the steps at all
    cluster_command = (
        cluster_commands[name].format(val)
        if val is not None
        else "/RMG/Output/Germanium/Cluster/PreClusterOutputs false"
    )

    # run the simulation
    run_sim(
        generator_name=generator,
        name=name,
        val=val,
        cluster=cluster_command,
        generator=config,
    )

    # post-process it
    run_reboost(
        generator_name=generator,
        name=name,
        val=val,
        reboost_config="config/hit_config.yaml",
    )

    # the output size has already been recorded in timing.json above, so the
    # step file can go: keeping all of them fills up the CI runner's disk
    if not keep_stp:
        shutil.rmtree(
            Path(f"out/{generator}/{name}/max_{val}/stp/"), ignore_errors=True
        )


if __name__ == "__main__":
    with Pool(n_proc) as pool:
        pool.map(run_sim_and_pproc, jobs)

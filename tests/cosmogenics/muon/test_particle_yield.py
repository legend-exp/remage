from __future__ import annotations

import multiprocessing as mp
import os
import time
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pyg4ometry as pg4
import pytest
from dbetto import TextDB
from lgdo import Scalar, lh5, types
from lgdo.lh5.concat import lh5concat
from pygeomhpges.materials import make_enriched_germanium
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

####################### SIMULATION #######################

HEIGHT_IN_G_OVER_CM2 = 2000
RADIUS_IN_CM = 150
N_EVENTS = 10000
SHARD_TIMEOUT_S = 3 * 60 * 60

DENSITIES = TextDB("./misc/")["densities"]

AVERAGE_A = {
    "lar": 40.0,
    "water": 18.0,
    "rock": 22.0,
    "enrGe": 72.6,
}

macro = """
/RMG/Processes/HadronicPhysics {had_physics}
/RMG/Processes/LowEnergyEMPhysics {em_physics}
/RMG/Processes/DefaultProductionCut {production_cut} mm
/RMG/Processes/SensitiveProductionCut {production_cut} mm
/RMG/Output/ActivateOutputScheme Track
/RMG/Geometry/RegisterDetector Germanium detector 1

/run/initialize

/RMG/Output/Track/StoreSinglePrecisionEnergy
/RMG/Output/Track/StoreSinglePrecisionPosition

/RMG/Processes/Stepping/DaughterNucleusMaxLifetime 1 ms

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     0 0 {height} cm
/gps/direction    0 0 -1
/gps/particle     mu-
/gps/energy       {energy} GeV

/run/beamOn {events}
"""


def geometry(mat_name: str):
    reg = pg4.geant4.Registry()
    matreg = LegendMaterialRegistry(reg, enable_optical=False)

    if mat_name == "lar":
        mat = matreg.liquidargon
    elif mat_name == "water":
        mat = matreg.water
    elif mat_name == "rock":
        mat = matreg.rock
    elif mat_name == "enrGe":
        mat = make_enriched_germanium(reg, "enrGe", enrichment=0.92)
    else:
        msg = f"unknown material {mat_name}"
        raise ValueError(msg)

    height = HEIGHT_IN_G_OVER_CM2 / DENSITIES[mat_name]  # convert to cm

    world_s = pg4.geant4.solid.Tubs(
        "world",
        0,
        RADIUS_IN_CM,
        height,
        0,
        2 * np.pi,
        registry=reg,
        lunit="cm",
    )
    world_l = pg4.geant4.LogicalVolume(world_s, mat, "world", registry=reg)
    reg.setWorld(world_l)

    safety = 1e-5
    detector_s = pg4.geant4.solid.Tubs(
        "detector",
        0,
        RADIUS_IN_CM - safety,
        height - safety,
        0,
        2 * np.pi,
        registry=reg,
        lunit="cm",
    )
    detector_l = pg4.geant4.LogicalVolume(detector_s, mat, "detector", registry=reg)
    pg4.geant4.PhysicalVolume(
        [0, 0, 0], [0, 0, 0], detector_l, "detector", world_l, registry=reg
    )

    return reg


def _case_output_name(
    energy: float,
    material: str,
    had_physics: str,
    em_physics: str,
    production_cut: int,
    shard_id: int | None = None,
) -> str:
    suffix = "" if shard_id is None else f"-shard{shard_id:03d}"
    return (
        f"output-neutron_yield-{energy:.0f}-{material}-{had_physics}-{em_physics}-"
        f"{production_cut}mm{suffix}.lh5"
    )


def simulate_worker(
    case: tuple[float, str, str, str, int],
    events: int | None,
    shard_id: int,
    result_queue: mp.Queue,
) -> None:
    try:
        energy, material, had_physic, em_physic, production_cut = case

        output_file = _case_output_name(
            energy, material, had_physic, em_physic, production_cut, shard_id=shard_id
        )

        height = HEIGHT_IN_G_OVER_CM2 / DENSITIES[material]

        remage_run(
            macro.split("\n"),
            macro_substitutions={
                "energy": energy,
                "events": events,
                "had_physics": had_physic,
                "em_physics": em_physic,
                "height": height / 2,
                "production_cut": production_cut,
            },
            gdml_files=geometry(material),
            output=output_file,
            overwrite_output=True,
            log_level="summary",
            threads=1,
            merge_output_files=False,
        )

        key, output = (
            (energy, material, had_physic, em_physic, production_cut),
            output_file,
        )
        result_queue.put(("ok", key, output, events))
    except Exception as exc:
        result_queue.put(("error", case, str(exc), events))


def _merge_case_outputs(
    case: tuple[float, str, str, str, int],
    shard_outputs: list[str],
) -> str | None:
    if not shard_outputs:
        return None

    merged_output = _case_output_name(*case)
    merged_path = Path(merged_output)
    if merged_path.exists():
        merged_path.unlink()

    if len(shard_outputs) == 1:
        Path(shard_outputs[0]).replace(merged_path)
        return str(merged_path)

    def _read_process_map(file_path: str) -> dict[str, int]:
        if lh5.ls(file_path, "processes") == []:
            return {}

        procs = lh5.read("processes", file_path)
        return {name: int(proc.value) for name, proc in procs.items()}

    combined_proc_map = {}
    for lh5_file in shard_outputs:
        proc_map = _read_process_map(lh5_file)
        for name, id in proc_map.items():
            if name not in combined_proc_map:
                combined_proc_map[name] = id

    # Scalar metadata groups are not concatenable; merge payload tables first, then restore.
    lh5concat(
        lh5_files=shard_outputs,
        output=str(merged_path),
        overwrite=True,
        exclude_list=[
            "detector_origins/*",
            "number_of_events",
            "processes/*",
        ],
    )

    process_struct = types.Struct(
        {name: Scalar(np.int64(pid)) for name, pid in combined_proc_map.items()}
    )
    lh5.write(process_struct, "processes", str(merged_path), wo_mode="overwrite")

    total_events = 0
    any_events = False
    for file in shard_outputs:
        if lh5.ls(file, "number_of_events") == []:
            continue
        total_events += int(lh5.read("number_of_events", file).value)
        any_events = True
    if any_events:
        lh5.write(
            Scalar(np.int64(total_events)),
            "number_of_events",
            str(merged_path),
            wo_mode="overwrite",
        )

    for file in shard_outputs:
        p = Path(file)
        if p.exists():
            p.unlink()

    return str(merged_path)


def run_cases_with_timeout(
    cases: list[tuple[float, str, str, str, int]],
    events: int | None = None,
    timeout_s: int = SHARD_TIMEOUT_S,
) -> dict[tuple[float, str, str, str, int], dict[str, str | int]]:
    outfiles = {}
    if not cases:
        return outfiles

    total_events = N_EVENTS if events is None else events
    if total_events <= 0:
        return outfiles

    cpu_half = max(1, (os.cpu_count() or 1) // 2)
    max_workers = max(1, cpu_half)
    per_case_shards = max(1, 2 * max_workers)

    # split cases into shards of small parts to isolate events with long runtime
    case_to_shards = {}
    for case in cases:
        n_shards = min(per_case_shards, total_events)
        base = total_events // n_shards
        rem = total_events % n_shards
        case_to_shards[case] = list(
            enumerate([base + (1 if i < rem else 0) for i in range(n_shards)])
        )

    # schedule shards
    pending_jobs = []
    max_num_shards = max(len(shards) for shards in case_to_shards.values())
    for shard_pos in range(max_num_shards):
        for case in cases:
            shards = case_to_shards[case]
            if shard_pos < len(shards):
                shard_id, shard_ev = shards[shard_pos]
                pending_jobs.append((case, shard_id, shard_ev))

    remaining_shards = {case: len(case_to_shards[case]) for case in cases}
    successful_outputs = {case: [] for case in cases}
    accepted_events = dict.fromkeys(cases, 0)
    merged_done = dict.fromkeys(cases, False)

    active_jobs = []
    while pending_jobs or active_jobs:  # processing loop
        while (
            pending_jobs and len(active_jobs) < max_workers
        ):  # start new processes when resources available
            case, shard_id, shard_ev = pending_jobs.pop(0)
            result_queue: mp.Queue = mp.Queue(maxsize=1)
            proc = mp.Process(
                target=simulate_worker,
                args=(case, shard_ev, shard_id, result_queue),
            )
            proc.start()
            active_jobs.append(
                {
                    "case": case,
                    "proc": proc,
                    "queue": result_queue,
                    "start": time.time(),
                    "shard_id": shard_id,
                    "events": shard_ev,
                }
            )

        remaining_active = []
        for job in (
            active_jobs
        ):  # check which shard are still active, perform merging if all shards complete
            case = job["case"]
            proc: mp.Process = job["proc"]
            result_queue: mp.Queue = job["queue"]
            runtime_s = time.time() - job["start"]

            job_finished = False
            if proc.is_alive() and runtime_s > timeout_s:
                print(
                    f"Timeout after {timeout_s}s for case={case}, shard={job['shard_id']}. "
                    "Terminating worker."
                )
                proc.terminate()
                proc.join()
                job_finished = True
            elif proc.is_alive():
                remaining_active.append(job)
                continue
            elif proc.exitcode != 0:
                print(
                    f"Worker exited with code {proc.exitcode} for case={case}, "
                    f"shard={job['shard_id']}"
                )
                job_finished = True
            elif result_queue.empty():
                print(f"No result returned for case={case}, shard={job['shard_id']}")
                job_finished = True
            else:
                status, _, payload, shard_ev = result_queue.get()
                if status == "ok" and Path(payload).is_file():
                    successful_outputs[case].append(payload)
                    accepted_events[case] += shard_ev
                elif status == "error":
                    print(
                        f"Simulation error for case={case}, shard={job['shard_id']}: {payload}"
                    )
                else:
                    print(
                        f"Missing output file for case={case}, shard={job['shard_id']}: {payload}"
                    )
                job_finished = True

            if job_finished:
                remaining_shards[case] -= 1
                if (
                    remaining_shards[case] == 0 and not merged_done[case]
                ):  # merging of shards when all processes done
                    merged_output = _merge_case_outputs(case, successful_outputs[case])
                    if merged_output is None or not Path(merged_output).is_file():
                        print(f"No valid merged output for case={case}")
                    else:
                        outfiles[case] = {
                            "path": merged_output,
                            "events": accepted_events[case],
                        }
                    merged_done[case] = True

        active_jobs = remaining_active
        if active_jobs:
            time.sleep(0.2)

    return outfiles


def _build_cases(
    energies: list[float],
    materials: list[str],
    had_physics_list: list[str],
    em_physics_list: list[str],
    production_cut_list: list[int],
) -> list[tuple[float, str, str, str, int]]:
    return [
        (energy, material, had_physic, em_physic, prod_cut)
        for material in materials
        for had_physic in had_physics_list
        for em_physic in em_physics_list
        for energy in energies
        for prod_cut in production_cut_list
    ]


TEST_CASE_CONFIGS = {
    "hadronic_physics_list_effect": {
        "energies": [273.0],
        "materials": ["lar"],
        "had_physics_list": [
            "Shielding",
            "QGSP_BIC_HP",
            "QGSP_BERT_HP",
            "FTFP_BERT_HP",
            "None",
        ],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
    },
    "material_effect": {
        "energies": [273.0],
        "materials": ["lar", "rock", "water"],
        "had_physics_list": ["Shielding"],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
    },
    "energy_dependence": {
        "energies": [100.0, 273.0, 1000.0],
        "materials": ["lar"],
        "had_physics_list": ["Shielding"],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
    },
}


@pytest.fixture(scope="module")
def precomputed_outfiles():
    all_cases: list[tuple[float, str, str, str, int]] = []
    for cfg in TEST_CASE_CONFIGS.values():
        all_cases.extend(_build_cases(**cfg))

    # Keep insertion order while removing duplicated case tuples.
    unique_cases = list(dict.fromkeys(all_cases))
    return run_cases_with_timeout(unique_cases)


####################### Plotter and Analysis #######################


def generate_proc_lookup_tables(procs):
    keys = procs.keys()
    values = [proc.value for proc in procs.values()]
    return dict(zip(keys, values, strict=True)), dict(zip(values, keys, strict=True))


def get_parent_particle(tracks, evtid, parent_trackid):
    mask_evtid = tracks["evtid"].view_as("np") == evtid
    mask_parent = tracks["trackid"].view_as("np")[mask_evtid] == parent_trackid
    return tracks["particle"].view_as("np")[mask_evtid][mask_parent][0]


def count_neutrons_per_interaction(tracks, id_to_name):
    evtid = tracks["evtid"].view_as("np")
    parent = tracks["parent_trackid"].view_as("np")
    particle = tracks["particle"].view_as("np")
    procid = tracks["procid"].view_as("np")

    # Group key: (evtid, parent_trackid)
    keys = np.stack((evtid, parent), axis=1)
    unique_keys, first_idx, inv = np.unique(
        keys, axis=0, return_index=True, return_inverse=True
    )

    is_neutron = (particle == 2112).astype(np.int64)
    neutron_counts_arr = np.bincount(
        inv, weights=is_neutron, minlength=len(unique_keys)
    ).astype(np.int64)

    group_procid = procid[first_idx]

    out = {}
    for g in np.flatnonzero(neutron_counts_arr > 0):
        evt, par = unique_keys[g]
        out[(int(evt), int(par))] = (
            int(neutron_counts_arr[g]),
            id_to_name[int(group_procid[g])],
            get_parent_particle(tracks, evt, par),
        )

    return out


def expected_neutron_yield(energy, material):
    data = TextDB("misc")["neutron_yield"]
    return (
        data["energy_dependence"][material]["b"]
        * energy ** data["energy_dependence"][material]["n_eff"]
    )


def calculate_neutron_yield(n_mean, n_std, n_muons, material, distance):

    val = n_mean / n_muons / (DENSITIES[material] * distance)
    std = n_std / n_muons / (DENSITIES[material] * distance)
    return {"val": val, "std": std}


def calculate_neutron_yield_from_sim_data(key, outfiles):
    (_, material, _, _, _) = key
    outfile_info = outfiles.get(key)
    if outfile_info is None:
        return None
    remage_output = outfile_info["path"]
    n_muons = outfile_info["events"]
    if remage_output is None or not Path(remage_output).is_file():
        return None
    if n_muons <= 0:
        return None

    tracks = lh5.read("tracks", remage_output)
    procs = lh5.read("processes", remage_output)

    _, id_to_name = generate_proc_lookup_tables(procs)

    ns_per_interactions = count_neutrons_per_interaction(tracks, id_to_name)

    n_batches = 100
    ns_produced = np.zeros(n_batches, dtype=int)
    max_evtid = np.max(tracks["evtid"].view_as("np"))
    batch_ranges = np.linspace(0, max_evtid + 1, n_batches + 1, dtype=int)
    for (evtid, _), (
        n_neutrons,
        proc_name,
        _,
    ) in ns_per_interactions.items():
        batch_idx = np.searchsorted(batch_ranges, evtid, side="right") - 1
        ns_produced[batch_idx] += n_neutrons
        if proc_name == "neutronInelastic":
            # Subtract the incoming neutron counted in this interaction.
            ns_produced[batch_idx] -= 1

    mean_bs_means = np.mean(ns_produced) * n_batches
    std_bs_means = np.std(ns_produced) * np.sqrt(n_batches)

    height = HEIGHT_IN_G_OVER_CM2 / DENSITIES[material]

    return calculate_neutron_yield(
        mean_bs_means, std_bs_means, n_muons, material, height
    )


def plot_neutron_yield_energy(
    energy_range, material, had_physic, em_physic, production_cut, outfiles
):
    neutron_yields = {}
    for energy in energy_range:
        key = (energy, material, had_physic, em_physic, production_cut)
        tmp = calculate_neutron_yield_from_sim_data(key, outfiles)
        if tmp is not None:
            neutron_yields[energy] = tmp

    fig, ax = plt.subplots()
    x_energy = list(neutron_yields.keys())
    y_yield = [v["val"] for v in neutron_yields.values()]
    y_yield_unc = [v["std"] for v in neutron_yields.values()]

    ax.errorbar(
        x_energy,
        y_yield,
        yerr=y_yield_unc,
        marker=".",
        lw=0,
        elinewidth=1,
        capsize=5,
        label="simulated yield",
    )

    x_yield_expected = np.linspace(min(x_energy), max(x_energy), 100)
    y_yield_expected = expected_neutron_yield(x_yield_expected, material)
    ax.plot(
        x_yield_expected,
        y_yield_expected,
        ls=":",
        color="black",
        label="expected yield",
    )

    ax.set_ylim(0, np.max([v["val"] for v in neutron_yields.values()]) * 1.25)
    ax.set_ylabel("neutron yield [n/muon/(g/cm^2)]")
    ax.set_xlabel("muon energy [GeV]")
    ax.set_title(
        f"neutron yield for muons in {material} with {had_physic} hadronic physics"
    )
    ax.legend()
    fig.savefig(
        f"neutron_yield_energy_scan_{material}_{had_physic}_{em_physic}_{production_cut}.output.png",
        bbox_inches="tight",
    )
    fig.clf()


def plot_neutron_yield_had_physic(
    had_physic_list_range, energy, material, em_physic, production_cut, outfiles
):

    neutron_yields = {}
    for had_physic in had_physic_list_range:
        key = (energy, material, had_physic, em_physic, production_cut)
        tmp = calculate_neutron_yield_from_sim_data(key, outfiles)
        if tmp is not None:
            neutron_yields[had_physic] = tmp

    fig, ax = plt.subplots()
    x_had_physic = list(neutron_yields.keys())
    y_yield = [v["val"] for v in neutron_yields.values()]
    y_yield_unc = [v["std"] for v in neutron_yields.values()]

    ax.errorbar(
        x_had_physic,
        y_yield,
        yerr=y_yield_unc,
        marker=".",
        lw=0,
        elinewidth=1,
        capsize=5,
        label="simulated yield",
    )

    y_yield_expected = expected_neutron_yield(energy, material)
    ax.axhline(
        y_yield_expected,
        ls=":",
        color="black",
        label="expected yield",
    )

    ax.set_ylim(0, np.max([v["val"] for v in neutron_yields.values()]) * 1.25)
    ax.set_ylabel("neutron yield [n/muon/(g/cm^2)]")
    ax.set_xlabel("hadronic physics list")
    ax.set_title(f"neutron yield for muons in {material} with {energy} GeV energy")
    ax.legend()
    fig.savefig(
        f"neutron_yield_had_physics_scan_{energy}_{material}_{em_physic}_{production_cut}.output.png",
        bbox_inches="tight",
    )
    fig.clf()


def plot_neutron_yield_material(
    material_range, energy, had_physic, em_physic, production_cut, outfiles
):

    neutron_yields = {}
    for material in material_range:
        key = (energy, material, had_physic, em_physic, production_cut)
        tmp = calculate_neutron_yield_from_sim_data(key, outfiles)
        if tmp is not None:
            neutron_yields[material] = tmp

    fig, ax = plt.subplots()
    x_material = [AVERAGE_A[mat] for mat in neutron_yields]
    y_yield = [v["val"] for v in neutron_yields.values()]
    y_yield_unc = [v["std"] for v in neutron_yields.values()]

    ax.errorbar(
        x_material,
        y_yield,
        yerr=y_yield_unc,
        marker=".",
        lw=0,
        elinewidth=1,
        capsize=5,
        label="simulated yield",
    )

    x_material_expected = [AVERAGE_A[mat] for mat in neutron_yields]
    y_yield_expected = [
        expected_neutron_yield(energy, mat) for mat in x_material_expected
    ]
    ax.plot(
        x_material_expected,
        y_yield_expected,
        ls=None,
        marker="x",
        color="black",
        label="expected yield",
    )
    ax.set_ylim(0, np.max([v["val"] for v in neutron_yields.values()]) * 1.25)
    ax.set_ylabel("neutron yield [n/muon/(g/cm^2)]")
    ax.set_xlabel("average A of material")
    ax.set_title(
        f"neutron yield for muons with {had_physic} hadronic physics and {energy} GeV energy"
    )
    ax.legend()
    fig.savefig(
        f"neutron_yield_material_scan_{energy}_{had_physic}_{em_physic}_{production_cut}.output.png",
        bbox_inches="tight",
    )
    fig.clf()


####################### Test on cases #######################


def test_hadronic_physics_list_effect(precomputed_outfiles):

    cfg = TEST_CASE_CONFIGS["hadronic_physics_list_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]

    plot_neutron_yield_had_physic(
        had_physic_list_range=had_physics_list,
        energy=energies[0],
        material=materials[0],
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        outfiles=precomputed_outfiles,
    )


def test_material_effect(precomputed_outfiles):

    cfg = TEST_CASE_CONFIGS["material_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]

    plot_neutron_yield_material(
        material_range=materials,
        energy=energies[0],
        had_physic=had_physics_list[0],
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        outfiles=precomputed_outfiles,
    )


def test_energy_dependence(precomputed_outfiles):

    cfg = TEST_CASE_CONFIGS["energy_dependence"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]

    plot_neutron_yield_energy(
        energy_range=energies,
        material=materials[0],
        had_physic=had_physics_list[0],
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        outfiles=precomputed_outfiles,
    )

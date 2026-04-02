from __future__ import annotations

import multiprocessing as mp
import os
import time
from pathlib import Path

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
import pyg4ometry as pg4
import pytest
from dbetto import TextDB
from lgdo import Scalar, lh5, types
from lgdo.lh5.concat import lh5concat
from matplotlib.colors import LogNorm
from pygeomhpges.materials import make_enriched_germanium
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

####################### SIMULATION #######################

HEIGHT_IN_G_OVER_CM2 = 2000
RADIUS_IN_CM = 125
N_EVENTS = 20000
SHARD_TIMEOUT_S = 6 * 60

DENSITIES = TextDB("./misc/")["densities"]

AVERAGE_A = {
    "lar": 40.0,
    "water": 14.335,
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
/RMG/Geometry/SetEkinMinForParticle {ekinmin_cut} MeV detector e-
/RMG/Geometry/SetEkinMinForParticle {ekinmin_cut} MeV detector e+
/RMG/Geometry/SetEkinMinForParticle {ekinmin_cut} MeV detector gamma

/run/initialize

/RMG/Output/Track/StoreSinglePrecisionEnergy
/RMG/Output/Track/StoreSinglePrecisionPosition

/RMG/Output/Germanium/StoreSinglePrecisionEnergy
/RMG/Output/Germanium/StoreSinglePrecisionPosition

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
    ekinmin_cut: float,
    shard_id: int | None = None,
) -> str:
    suffix = "" if shard_id is None else f"-shard{shard_id:03d}"
    return (
        f"output-neutron_yield-{energy:.0f}-{material}-{had_physics}-{em_physics}-"
        f"{production_cut}mm-{ekinmin_cut:.1f}MeV{suffix}.lh5"
    )


def simulate_worker(
    case: tuple[float, str, str, str, int, float],
    events: int | None,
    shard_id: int,
    result_queue: mp.Queue,
) -> None:
    try:
        energy, material, had_physic, em_physic, production_cut, ekinmin_cut = case

        output_file = _case_output_name(
            energy,
            material,
            had_physic,
            em_physic,
            production_cut,
            ekinmin_cut,
            shard_id=shard_id,
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
                "ekinmin_cut": ekinmin_cut,
            },
            gdml_files=geometry(material),
            output=output_file,
            overwrite_output=True,
            log_level="summary",
            threads=1,
            merge_output_files=False,
        )

        key, output = (
            (energy, material, had_physic, em_physic, production_cut, ekinmin_cut),
            output_file,
        )
        result_queue.put(("ok", key, output, events))
    except Exception as exc:
        result_queue.put(("error", case, str(exc), events))


def _merge_case_outputs(
    case: tuple[float, str, str, str, int, float],
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
        out = {}
        # Support both the old format (Struct of Scalars) and the new format (Struct with 'name' and 'procid' arrays)
        if "name" in procs and "procid" in procs:
            print(
                f"Reading process map from file '{file_path}' using new format with 'name' and 'procid' arrays"
            )
            names = procs["name"].nda
            procids = procs["procid"].nda
            for n, pid in zip(names, procids, strict=False):
                # Ensure the name is decoded to string if it is bytes
                name_str = n.decode("utf-8") if isinstance(n, bytes) else str(n)
                out[name_str] = int(pid)
        else:
            # print(f"Warning: 'processes' in file '{file_path}' does not contain 'name' and 'procid' arrays, falling back to old format")
            for name, proc in procs.items():
                val = (
                    proc.value
                    if hasattr(proc, "value")
                    else proc.nda[0]
                    if hasattr(proc, "nda") and getattr(proc.nda, "ndim", 0) > 0
                    else getattr(proc, "nda", proc)
                )
                out[name] = int(val)
        return out

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
        ev_obj = lh5.read("number_of_events", file)
        ev_val = (
            ev_obj.value
            if hasattr(ev_obj, "value")
            else ev_obj.nda[0]
            if hasattr(ev_obj, "nda") and getattr(ev_obj.nda, "ndim", 0) > 0
            else getattr(ev_obj, "nda", ev_obj)
        )
        total_events += int(ev_val)
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
    cases: list[tuple[float, str, str, str, int, float]],
    events: int | None = None,
    timeout_s: int = SHARD_TIMEOUT_S,
    files_already_processed: bool = False,
) -> dict[tuple[float, str, str, str, int, float], dict[str, str | int]]:
    outfiles = {}
    if not cases:
        return outfiles

    total_events = N_EVENTS if events is None else events
    if total_events <= 0:
        return outfiles

    cpu_usage = max(1, int((os.cpu_count() or 1) * 0.75))
    max_workers = max(1, cpu_usage)
    per_case_shards = 100  # max(1, 2 * max_workers)

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

    if files_already_processed:
        outfiles = {}
        for case in cases:
            sucessfull_shard_outputs = []
            accepted_case_events = 0
            for shard_id, shard_ev in case_to_shards[case]:
                shard_output = _case_output_name(*case, shard_id=shard_id)
                if Path(shard_output).is_file():
                    sucessfull_shard_outputs.append(shard_output)
                    accepted_case_events += shard_ev
            merged_output = _merge_case_outputs(case, sucessfull_shard_outputs)
            if merged_output and Path(merged_output).is_file():
                outfiles[case] = {
                    "path": merged_output,
                    "events": accepted_case_events,
                }
        return outfiles

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
                            "fraction_accepted": accepted_events[case] / total_events,
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
    ekinmin_cuts: list[float],
) -> list[tuple[float, str, str, str, int, float]]:
    if len(ekinmin_cuts) not in (
        1,
        len(energies),
        len(materials),
        len(had_physics_list),
        len(em_physics_list),
        len(production_cut_list),
    ):
        msg = f"Length of ekinmin_cuts must be either 1 or equal to the number of energies ({len(energies)}), but got {len(ekinmin_cuts)}"
        raise ValueError(msg)
    tmp = [
        (energy, material, had_physic, em_physic, prod_cut)
        for material in materials
        for had_physic in had_physics_list
        for em_physic in em_physics_list
        for energy in energies
        for prod_cut in production_cut_list
    ]
    cases = []
    for idx, (energy, material, had_physic, em_physic, prod_cut) in enumerate(tmp):
        ekinmin_cut = ekinmin_cuts[idx % len(ekinmin_cuts)]
        cases.append((energy, material, had_physic, em_physic, prod_cut, ekinmin_cut))
    return cases


TEST_CASE_CONFIGS = {
    "hadronic_physics_list_effect": {
        "energies": [100.0],
        "materials": ["lar"],
        "had_physics_list": [
            "Shielding",
            "QGSP_BIC_HP",
            "QGSP_BERT_HP",
            "FTFP_BERT_HP",
        ],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
        "ekinmin_cuts": [9.869],
    },
    "material_effect": {
        "energies": [100.0],
        "materials": ["lar", "rock", "water"],
        "had_physics_list": ["Shielding"],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
        "ekinmin_cuts": [9.869, 8.045, 8.045],
    },
    "energy_dependence_lar": {
        "energies": [100.0, 280.0],
        "materials": ["lar"],
        "had_physics_list": ["Shielding"],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
        "ekinmin_cuts": [9.869],
    },
    "energy_dependence_water": {
        "energies": [100.0, 280.0],
        "materials": ["water"],
        "had_physics_list": ["Shielding"],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
        "ekinmin_cuts": [8.045],
    },
}


@pytest.fixture(scope="module")
def precomputed_outfiles():
    all_cases: list[tuple[float, str, str, str, int, float]] = []
    for cfg in TEST_CASE_CONFIGS.values():
        all_cases.extend(_build_cases(**cfg))

    # Keep insertion order while removing duplicated case tuples.
    unique_cases = list(dict.fromkeys(all_cases))
    output = run_cases_with_timeout(unique_cases, files_already_processed=False)

    print("\nSimulation outputs:")
    for case, info in output.items():
        print("Case:", case)
        print("  Output file:", info["path"])
        print("  Events in output:", info["events"])
        print("  Fraction of events accepted:", info["fraction_accepted"])

    return output


####################### Plotter and Analysis #######################


def generate_proc_lookup_tables(procs):
    keys = list(procs.keys())
    if "name" in keys and "procid" in keys:
        names = procs["name"].nda
        procids = procs["procid"].nda
        id_to_name = {}
        name_to_id = {}
        for name, procid in zip(names, procids, strict=False):
            name_str = name.decode("utf-8") if isinstance(name, bytes) else str(name)
            pid = int(procid)
            id_to_name[pid] = name_str
            name_to_id[name_str] = pid
        return name_to_id, id_to_name

    values = [int(proc.value) for proc in procs.values()]
    name_to_id = dict(zip(keys, values, strict=False))
    id_to_name = dict(zip(values, keys, strict=False))
    return name_to_id, id_to_name


def _build_segment_ids(evtids):
    evtids = np.asarray(evtids, dtype=np.int64)
    segments = np.zeros(evtids.shape, dtype=np.int64)
    if evtids.size > 1:
        segments[1:] = np.cumsum(evtids[1:] < evtids[:-1], dtype=np.int64)
    return segments


def aggregate_neutron_interactions(tracks):
    evtid = tracks["evtid"].view_as("np").astype(np.int64, copy=False)
    parent = tracks["parent_trackid"].view_as("np").astype(np.int64, copy=False)
    particle = tracks["particle"].view_as("np").astype(np.int64, copy=False)
    procid = tracks["procid"].view_as("np").astype(np.int64, copy=False)
    xloc = tracks["xloc"].view_as("np").astype(np.float32, copy=False)
    yloc = tracks["yloc"].view_as("np").astype(np.float32, copy=False)
    zloc = tracks["zloc"].view_as("np").astype(np.float32, copy=False)
    segments = _build_segment_ids(evtid)
    group_keys = np.stack((segments, evtid, parent), axis=1)

    _, first_idx, inv = np.unique(
        group_keys,
        axis=0,
        return_index=True,
        return_inverse=True,
    )

    neutron_counts = np.bincount(
        inv,
        weights=(particle == 2112).astype(np.int64),
        minlength=first_idx.size,
    ).astype(np.int64)

    nz = neutron_counts > 0
    return (
        segments[first_idx][nz],
        evtid[first_idx][nz],
        parent[first_idx][nz],
        procid[first_idx][nz],
        xloc[first_idx][nz],
        yloc[first_idx][nz],
        zloc[first_idx][nz],
        neutron_counts[nz],
    )


def expected_neutron_yield(material, name="Manukovsky_et_al_2016", energy=None):
    data = TextDB("./misc")["neutron_yield"][name]["data"]
    if "energy_dependence" in data:
        if energy is None:
            msg = f"Energy must be provided for dataset '{name}' with energy dependence"
            raise ValueError(msg)
        E = energy
        Y_gen = (
            data["energy_dependence"][material]["b"]
            * energy ** data["energy_dependence"][material]["n_eff"]
        )
        Y_gen_unc = 0
    elif material in data["Y_gen"]:
        E = next(iter(data["Y_gen"][material].keys()))
        Y_gen = data["Y_gen"][material][E]["val"]
        Y_gen_unc = data["Y_gen"][material][E]["std"]
    else:
        msg = f"No expected yield data for material '{material}' in dataset '{name}'"
        raise ValueError(msg)
    return {
        "energy": E,
        "yield": {"val": Y_gen, "std": Y_gen_unc},
    }


def calculate_neutron_yield(n_mean, n_std, n_muons, material, distance):

    val = n_mean / n_muons / (DENSITIES[material] * distance)
    std = n_std / n_muons / (DENSITIES[material] * distance)
    return {"val": val, "std": std}


def bootstrap(n_produced, n_bootstrap=10000):
    n = len(n_produced)
    if n == 0:
        return 0.0, 0.0
    rng = np.random.default_rng()
    idx = rng.integers(0, n, size=(n_bootstrap, n))
    means = n_produced[idx].sum(axis=1)
    return float(np.mean(means)), float(np.std(means))


def calculate_neutron_yield_from_sim_data(key, outfiles):
    (_, material, _, _, _, _) = key
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
    name_to_id, _ = generate_proc_lookup_tables(procs)

    segment_ids, evtids, _, procids, _, _, z_n, neutron_counts = (
        aggregate_neutron_interactions(tracks)
    )
    if evtids.size == 0:
        return calculate_neutron_yield(
            0.0, 0.0, n_muons, material, HEIGHT_IN_G_OVER_CM2 / DENSITIES[material]
        )

    min_depth = 4  # m
    z_n = -z_n - np.min(z_n)
    mask_depth = z_n >= min_depth

    n_batches = 100
    max_local_evtid = int(np.max(tracks["evtid"].view_as("np")))
    global_evtids = segment_ids.astype(np.int64) * (
        max_local_evtid + 1
    ) + evtids.astype(np.int64)
    max_global_evtid = int(np.max(global_evtids))
    batch_ranges = np.linspace(0, max_global_evtid + 1, n_batches + 1, dtype=np.int64)
    batch_idx = np.searchsorted(batch_ranges, global_evtids, side="right") - 1
    batch_idx = np.clip(batch_idx, 0, n_batches - 1)

    ns_produced = np.bincount(
        batch_idx[mask_depth],
        weights=neutron_counts.astype(np.int64)[mask_depth],
        minlength=n_batches,
    ).astype(np.int64)

    neutron_inelastic_id = name_to_id.get("neutronInelastic")
    if neutron_inelastic_id is not None:
        inelastic_mask = procids == int(neutron_inelastic_id)
        if np.any(inelastic_mask):
            ns_produced -= np.bincount(
                batch_idx[mask_depth & inelastic_mask],
                minlength=n_batches,
            ).astype(np.int64)
    mean_bs_means, std_bs_means = bootstrap(ns_produced)
    height = HEIGHT_IN_G_OVER_CM2 / DENSITIES[material] - min_depth * 100

    return calculate_neutron_yield(
        mean_bs_means, std_bs_means, n_muons, material, height
    )


def plot_neutron_yield_energy(
    energy_range, material, had_physic, em_physic, production_cut, ekinmin_cut, outfiles
):
    neutron_yields = {}
    for energy in energy_range:
        key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
        print(key)
        tmp = calculate_neutron_yield_from_sim_data(key, outfiles)
        if tmp is not None:
            neutron_yields[energy] = tmp

    print("neutron_yields: ", neutron_yields)
    fig, ax = plt.subplots()
    x_energy = list(neutron_yields.keys())
    y_yield = [v["val"] for v in neutron_yields.values()]
    y_yield_unc = [v["std"] for v in neutron_yields.values()]

    print(f"x_energy: {x_energy}")
    print(f"y_yield: {y_yield}+-{y_yield_unc}")

    ax.errorbar(
        x_energy,
        y_yield,
        yerr=y_yield_unc,
        marker=".",
        lw=0,
        elinewidth=1,
        capsize=5,
        label="remage simulation",
        zorder=5,
    )

    energy = np.linspace(0, 400, 101)  # np.linspace(min(x_energy), max(x_energy), 100)
    n_yield_Manukovsky = expected_neutron_yield(
        material, name="Manukovsky_et_al_2016", energy=energy
    )

    x_yield_expected = np.array(list(n_yield_Manukovsky["energy"]))
    y_yield_expected = np.array(list(n_yield_Manukovsky["yield"]["val"]))

    ax.plot(
        x_yield_expected,
        y_yield_expected,
        marker=None,
        color="black",
        label="[10] (G4.9.4, QGS-BiC, ENDF/B-VI)",
        zorder=4,
    )

    n_yield_Agafonova = expected_neutron_yield(
        material, name="UNIVERSAL_FORMULA_2013", energy=energy
    )

    x_yield_expected_uf = np.array(list(n_yield_Agafonova["energy"]))
    y_yield_expected_uf = np.array(list(n_yield_Agafonova["yield"]["val"]))

    ax.plot(
        x_yield_expected_uf,
        y_yield_expected_uf,
        ls=":",
        marker=None,
        color="black",
        label="[9] general parametrization",
        zorder=4,
    )

    if material == "water":
        n_yield_FLUKA_2001 = expected_neutron_yield(
            material, name="FLUKA_2001", energy=energy
        )

        x_yield_expected_FLUKA_2001 = np.array(list(n_yield_FLUKA_2001["energy"]))
        y_yield_expected_FLUKA_2001 = np.array(list(n_yield_FLUKA_2001["yield"]["val"]))

        ax.plot(
            x_yield_expected_FLUKA_2001,
            y_yield_expected_FLUKA_2001,
            ls="--",
            marker=None,
            color="grey",
            label="[7] (FLUKA, Wang et al. 2001)",
        )

        n_yield_FLUKA_2003 = expected_neutron_yield(
            material, name="FLUKA_2003", energy=energy
        )

        x_yield_expected_FLUKA_2003 = np.array(list(n_yield_FLUKA_2003["energy"]))
        y_yield_expected_FLUKA_2003 = np.array(list(n_yield_FLUKA_2003["yield"]["val"]))

        ax.plot(
            x_yield_expected_FLUKA_2003,
            y_yield_expected_FLUKA_2003,
            ls="-.",
            marker=None,
            color="grey",
            label="[8] (FLUKA, Kudryavtsev et al. 2003)",
        )

        n_yield_SK = expected_neutron_yield(material, name="SK_2023")
        ax.errorbar(
            n_yield_SK["energy"],
            n_yield_SK["yield"]["val"],
            yerr=n_yield_SK["yield"]["std"],
            marker=".",
            color="tab:orange",
            lw=0,
            elinewidth=1,
            capsize=5,
            label="[11] Super-Kamiokande",
        )

        n_yield_SNO = expected_neutron_yield(material, name="SNO_2026")
        ax.errorbar(
            n_yield_SNO["energy"],
            n_yield_SNO["yield"]["val"],
            yerr=n_yield_SNO["yield"]["std"],
            marker=".",
            color="tab:green",
            lw=0,
            elinewidth=1,
            capsize=5,
            label="[12] SNO+",
        )

    ax.set_ylim(
        0,
        np.max(
            np.concatenate(
                [
                    np.array([v["val"] for v in neutron_yields.values()]) * 1.25,
                    y_yield_expected * 1.25,
                ]
            )
        ),
    )
    ax.set_ylabel("neutron yield [n/muon/(g/cm^2)]")
    ax.set_xlabel("muon energy [GeV]")
    ax.set_xlim(0, 400)
    ax.set_title(
        f"neutron yield for muons in {material} with {had_physic} hadronic physics"
    )
    ax.legend()

    ax.grid(ls=":", color="black", alpha=0.5)
    fig.savefig(
        f"neutron_yield_energy_scan_{material}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )


def plot_neutron_yield_had_physic(
    had_physic_list_range,
    energy,
    material,
    em_physic,
    production_cut,
    ekinmin_cut,
    outfiles,
):

    neutron_yields = {}
    for had_physic in had_physic_list_range:
        key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
        tmp = calculate_neutron_yield_from_sim_data(key, outfiles)
        if tmp is not None:
            neutron_yields[had_physic] = tmp

    print("neutron_yields: ", neutron_yields)

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
        label="remage simulation",
    )

    y_yield_expected = expected_neutron_yield(
        material, name="Manukovsky_et_al_2016", energy=energy
    )["yield"]["val"]
    ax.axhline(
        y_yield_expected,
        color="black",
        label="[10] (G4.9.4, QGS-BiC, ENDF/B-VI)",
    )

    ax.set_ylim(
        0,
        np.max(
            np.concatenate(
                [
                    np.array([v["val"] for v in neutron_yields.values()]) * 1.25,
                    [y_yield_expected * 1.25],
                ]
            )
        ),
    )
    ax.set_ylabel("neutron yield [n/muon/(g/cm^2)]")
    ax.set_xlabel("hadronic physics list")
    ax.set_title(f"neutron yield for muons in {material} with {energy} GeV energy")
    ax.legend()

    ax.grid(ls=":", color="black", alpha=0.5)
    fig.savefig(
        f"neutron_yield_had_physics_scan_{energy}_{material}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )


def plot_neutron_yield_material(
    material_range,
    energy,
    had_physic,
    em_physic,
    production_cut,
    ekinmin_cut_range,
    outfiles,
):

    neutron_yields = {}
    for i, material in enumerate(material_range):
        ekinmin_cut = ekinmin_cut_range[i % len(ekinmin_cut_range)]
        key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
        tmp = calculate_neutron_yield_from_sim_data(key, outfiles)
        if tmp is not None:
            neutron_yields[material] = tmp
        else:
            print(
                f"No valid neutron yield calculated for material '{material}' with key {key}"
            )
    print("neutron_yields: ", neutron_yields)

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
        label="remage simulation",
    )

    x_material_expected = [AVERAGE_A[mat] for mat in neutron_yields]
    y_yield_expected = np.array(
        [
            expected_neutron_yield(mat, name="Manukovsky_et_al_2016", energy=energy)[
                "yield"
            ]["val"]
            for mat in neutron_yields
        ]
    )
    ax.plot(
        x_material_expected,
        y_yield_expected,
        lw=0,
        marker="x",
        color="black",
        label="[10] (G4.9.4, QGS-BiC, ENDF/B-VI)",
    )

    x_material_expected_uf_range = np.linspace(0, 50, 100)
    uf_data = TextDB("./misc")["neutron_yield"]["UNIVERSAL_FORMULA_2013"]["data"][
        "mass_dependence"
    ]
    y_yield_expected_uf_range = (
        uf_data["b"]
        * (energy ** uf_data["alpha"])
        * (x_material_expected_uf_range ** uf_data["beta"])
    )
    ax.plot(
        x_material_expected_uf_range,
        y_yield_expected_uf_range,
        ls=":",
        marker=None,
        color="black",
        label="[9] general parametrization",
    )

    for mat in material_range:
        ax.vlines(
            AVERAGE_A[mat],
            0,
            0.2 * np.min([v["val"] for v in neutron_yields.values()]),
            colors="gray",
            linestyles="dashed",
            alpha=0.5,
        )
        ax.text(
            AVERAGE_A[mat],
            0.2 * np.min([v["val"] for v in neutron_yields.values()]),
            mat,
            rotation=90,
            verticalalignment="bottom",
            horizontalalignment="left",
            fontsize=8,
            color="gray",
        )

    ax.set_ylim(
        0,
        np.max(
            np.concatenate(
                [
                    np.array([v["val"] for v in neutron_yields.values()]) * 1.25,
                    y_yield_expected * 1.25,
                ]
            )
        ),
    )
    ax.set_ylabel("neutron yield [n/muon/(g/cm^2)]")
    ax.set_xlabel("average A of material")
    # ax.set_xscale("log")
    ax.set_xlim(0, 50)
    ax.set_title(
        f"neutron yield for muons with {had_physic} hadronic physics and {energy} GeV energy"
    )

    ax.grid(ls=":", color="black", alpha=0.5)
    ax.legend()
    fig.savefig(
        f"neutron_yield_material_scan_{energy}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )


def plot_shower_dimensions(
    energy, material, had_physic, em_physic, production_cut, ekinmin_cut, outfiles
):
    key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
    outfile_info = outfiles.get(key)
    print(key, outfile_info)
    if outfile_info is None:
        print(f"No output file for key {key}")
        return
    tracks = lh5.read("tracks", outfile_info["path"])
    _, _, _, procids, x_n, y_n, z_n, neutron_counts = aggregate_neutron_interactions(
        tracks
    )

    stp = lh5.read("stp", outfile_info["path"])

    procs = lh5.read("processes", outfile_info["path"])
    name_to_id, _ = generate_proc_lookup_tables(procs)

    det001 = stp["det001"].view_as("ak")
    x = np.array(ak.ravel(det001.xloc))
    y = np.array(ak.ravel(det001.yloc))
    z = np.array(ak.ravel(det001.zloc))
    edep = np.array(ak.ravel(det001.edep))
    z_offset = np.min(z)
    z = -z - z_offset
    z_n = -z_n - z_offset

    bins_2d_radial = [
        np.linspace(np.min(z), np.max(z), 301),
        np.linspace(0, np.max(np.sqrt(x**2 + y**2)), 301),
    ]
    hist_energy_2d = np.histogram2d(
        z, np.sqrt(x**2 + y**2), bins=bins_2d_radial, weights=edep
    )
    perc_995_distance = np.array(
        [
            hist_energy_2d[2][
                np.argwhere(np.cumsum(x_slice) / np.sum(x_slice) >= 0.995)[0]
            ][0]
            if np.sum(x_slice) > 0
            else 0
            for x_slice in hist_energy_2d[0]
        ]
    )

    def estimate_fwhm(hist):
        bin_centers = (hist[1][:-1] + hist[1][1:]) / 2
        half_max = np.max(hist[0]) / 2
        mode_index = np.argmax(hist[0])
        left_index = np.where(hist[0][:mode_index] < half_max)[0][-1]
        right_index = np.where(hist[0][mode_index:] < half_max)[0][0] + mode_index
        return bin_centers[right_index] - bin_centers[left_index]

    hist2 = np.histogram(perc_995_distance, bins=np.linspace(0, 1.5, 101))
    bin_centers = (hist2[1][:-1] + hist2[1][1:]) / 2
    mode = bin_centers[np.argmax(hist2[0])]
    fwhm = estimate_fwhm(hist2)

    first_995_above_mode = bins_2d_radial[0][
        np.where(perc_995_distance > mode - fwhm / 2.0)[0][0]
    ]

    z_max_manual = 7

    fig, ax = plt.subplots(
        2,
        2,
        height_ratios=[3, 1],
        width_ratios=[3, 1],
        sharex="col",
        sharey="row",
        gridspec_kw={"hspace": 0.05, "wspace": 0.05},
    )
    ax = ax.flatten()
    ax[0].hist2d(
        z,
        np.sqrt(y**2 + x**2),
        bins=[
            np.linspace(np.min(z), z_max_manual, 101),
            np.linspace(0, np.max(np.sqrt(x**2 + y**2)), 101),
        ],
        weights=edep,
        norm=LogNorm(1e2, 1e11),
        cmap="Greys",
    )
    # fig.colorbar(c[3], label="energy deposition [MeV]")
    ax[0].plot(
        bins_2d_radial[0][:-1],
        perc_995_distance,
        color="tab:orange",
        label=r"99.5% energy containment (c$_{995}$) envelope",
    )
    ax[0].axvline(
        first_995_above_mode,
        color="tab:blue",
        ls="--",
        label=f"Shower stabilized at {first_995_above_mode:.2f} m",
    )
    ax[0].hlines(
        mode,
        xmin=first_995_above_mode,
        xmax=z_max_manual,
        color="tab:red",
        label=r"c$_{995}$"
        + f" width at stabilization: {mode:.2f}+-{fwhm / 2.335:.2f} m",
    )
    ax[0].axhspan(
        ymin=mode - fwhm / 2,
        ymax=mode + fwhm / 2,
        xmin=first_995_above_mode / z_max_manual,
        xmax=1,
        color="tab:red",
        alpha=0.2,
    )
    ax[0].set_xlim(0, z_max_manual)
    ax[0].set_ylabel("radial distance [m]")
    ax[0].set_title(f"Shower dimensions for {energy} GeV muons in {material}")
    ax[0].grid(ls=":", color="black", alpha=0.5)
    ax[0].legend()

    # neutron production profile
    hist_n_radial = np.histogram(
        np.sqrt(ak.ravel(x_n) ** 2 + ak.ravel(y_n) ** 2),
        weights=neutron_counts,
        bins=np.linspace(0, np.max(bins_2d_radial[1]), 101),
    )
    neutron_inelastic_procid = name_to_id.get("neutronInelastic")
    hist_n_radial_inelastic_reduced = np.histogram(
        np.sqrt(ak.ravel(x_n) ** 2 + ak.ravel(y_n) ** 2),
        weights=(procids == int(neutron_inelastic_procid)).astype(float),
        bins=np.linspace(0, np.max(bins_2d_radial[1]), 101),
    )
    hist_reduced = (hist_n_radial[0] - hist_n_radial_inelastic_reduced[0]) / N_EVENTS
    ax[1].hist(
        hist_n_radial[1][:-1],
        bins=hist_n_radial[1],
        weights=hist_reduced,
        orientation="horizontal",
        histtype="step",
        color="black",
        label="neutron production radial distribution",
    )
    # r_90_neutron_prod = bins_2d_radial[1][np.where(np.cumsum(hist_reduced)/np.sum(hist_reduced) >= 0.9)[0][0]]
    # ax[1].axhline(r_90_neutron_prod, color="grey", ls="--", label=f"90% neutron production within {r_90_neutron_prod:.2f} m radial distance")

    # ax[1].step((hist_n_radial[1][1:] + hist_n_radial[1][:-1]) / 2, hist_n_radial[0] - hist_n_radial_inelastic_reduced[0], orientation="horizontal", histtype="step", color="black", label="neutron production depth distribution")

    #    ax[1].hist(np.sqrt(ak.ravel(x_n)**2 + ak.ravel(y_n)**2), bins=np.linspace(0, np.max(bins_2d_radial[1]), 101), orientation="horizontal", histtype="step", color="black", label="neutron production depth distribution")
    ax[1].set_xscale("log")
    ax[1].set_xlabel("neutron prod.")
    ax[1].grid(ls=":", color="black", alpha=0.5)

    hist_n_z = np.histogram(
        ak.ravel(z_n), weights=neutron_counts, bins=np.linspace(0, z_max_manual, 101)
    )
    hist_n_z_inelastic_reduced = np.histogram(
        ak.ravel(z_n),
        weights=(procids == neutron_inelastic_procid).astype(float),
        bins=np.linspace(0, z_max_manual, 101),
    )
    ax[2].hist(
        hist_n_z[1][:-1],
        bins=hist_n_z[1],
        weights=(hist_n_z[0] - hist_n_z_inelastic_reduced[0]) / N_EVENTS,
        histtype="step",
        color="black",
        label="neutron production depth distribution",
    )

    #    ax[2].hist(ak.ravel(z_n), bins=np.linspace(0, z_max_manual, 101), histtype="step", color="black", label="neutron production depth distribution")
    ax[2].axvline(first_995_above_mode, color="tab:blue", ls="--")
    ax[2].set_xlabel("depth [m]")
    ax[2].set_ylabel("neutron prod.")
    ax[2].grid(ls=":", color="black", alpha=0.5)

    ax[3].set_visible(False)

    fig.savefig(
        f"shower_dimensions_{energy}_{material}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )


def plot_neutron_production_processes(
    energy, material, had_physic, em_physic, production_cut, ekinmin_cut, outfiles
):
    key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
    outfile_info = outfiles.get(key)
    print(key, outfile_info)
    if outfile_info is None:
        print(f"No output file for key {key}")
        return
    tracks = lh5.read("tracks", outfile_info["path"])
    procs = lh5.read("processes", outfile_info["path"])
    _, id_to_name = generate_proc_lookup_tables(procs)

    _, _, _, procids, _, _, _, neutron_counts = aggregate_neutron_interactions(tracks)

    unique_procids = np.unique(procids)
    total_pro_procid = {
        pid: np.sum(neutron_counts[(procids == pid)]) for pid in unique_procids
    }
    total_neutrons = np.sum(list(total_pro_procid.values()))
    procid_counts_per_mult = {
        pid: {
            mult: np.sum(neutron_counts[(procids == pid) & (neutron_counts == mult)])
            for mult in np.unique(neutron_counts[procids == pid])
        }
        for pid in unique_procids
        if total_pro_procid[pid] > total_neutrons * 1e-3
    }
    multiplicities = np.unique(neutron_counts)
    procid_counts_named_per_mult = {
        id_to_name.get(pid, str(pid)): count
        for pid, count in procid_counts_per_mult.items()
    }

    neutron_inelastic_counts = procid_counts_named_per_mult.get("neutronInelastic")
    if neutron_inelastic_counts is not None:
        tmp = neutron_inelastic_counts.copy()
        for mult in tmp:
            if mult == 1:
                continue
            neutron_inelastic_counts[mult - 1] = tmp[mult]

    fig, ax = plt.subplots()
    for mult in [1, 2, 3, 4, 5]:
        if mult < 5:
            counts_for_mult = [
                procid_counts_per_mult.get(pid, {}).get(mult, 0)
                for pid in procid_counts_per_mult
            ]
            ax.barh(
                procid_counts_named_per_mult.keys(),
                counts_for_mult,
                label=f"{mult}n",
                left=[
                    sum(
                        procid_counts_per_mult.get(pid, {}).get(m, 0)
                        for m in multiplicities
                        if m < mult
                    )
                    for pid in procid_counts_per_mult
                ],
            )
        else:
            counts_for_mult = [
                np.sum(
                    [
                        procid_counts_per_mult.get(pid, {}).get(m, 0)
                        for m in procid_counts_per_mult.get(pid, {})
                        if m >= mult
                    ]
                )
                for pid in procid_counts_per_mult
            ]
            ax.barh(
                procid_counts_named_per_mult.keys(),
                counts_for_mult,
                label=r"$\geq$" + f"{mult}n",
                left=[
                    sum(
                        procid_counts_per_mult.get(pid, {}).get(m, 0)
                        for m in multiplicities
                        if m < mult
                    )
                    for pid in procid_counts_per_mult
                ],
            )

    ax.set_xlabel("number of neutrons produced")
    ax.set_title(f"Neutron production processes for {energy} GeV muons in {material}")
    ax.grid(ls=":", color="black", alpha=0.5)
    ax.legend()
    fig.savefig(
        f"neutron_production_processes_{energy}_{material}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )


####################### Test on cases #######################


def test_hadronic_physics_list_effect(precomputed_outfiles):
    cfg = TEST_CASE_CONFIGS["hadronic_physics_list_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    print(
        "Running test_hadronic_physics_list_effect with hadronic physics lists: ",
        had_physics_list,
    )
    plot_neutron_yield_had_physic(
        had_physic_list_range=had_physics_list,
        energy=energies[0],
        material=materials[0],
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        ekinmin_cut=ekinmin_cuts[0],
        outfiles=precomputed_outfiles,
    )


def test_material_effect(precomputed_outfiles):
    cfg = TEST_CASE_CONFIGS["material_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    print("Running test_material_effect with materials: ", materials)
    plot_neutron_yield_material(
        material_range=materials,
        energy=energies[0],
        had_physic=had_physics_list[0],
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        ekinmin_cut_range=ekinmin_cuts,
        outfiles=precomputed_outfiles,
    )


def test_energy_dependence(precomputed_outfiles):
    cfg = TEST_CASE_CONFIGS["energy_dependence_lar"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    print("Running test_energy_dependence with energies: ", energies)
    plot_neutron_yield_energy(
        energy_range=energies,
        material=materials[0],
        had_physic=had_physics_list[0],
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        ekinmin_cut=ekinmin_cuts[0],
        outfiles=precomputed_outfiles,
    )

    cfg = TEST_CASE_CONFIGS["energy_dependence_water"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    print("Running test_energy_dependence with energies: ", energies)
    plot_neutron_yield_energy(
        energy_range=energies,
        material=materials[0],
        had_physic=had_physics_list[0],
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        ekinmin_cut=ekinmin_cuts[0],
        outfiles=precomputed_outfiles,
    )


def test_shower_dimensions(precomputed_outfiles):
    cfg = TEST_CASE_CONFIGS["material_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    print("Running test_shower_dimensions with materials: ", materials)
    for i, material in enumerate(materials):
        ekinmin_cut = ekinmin_cuts[i % len(ekinmin_cuts)]
        plot_shower_dimensions(
            energy=energies[0],
            material=material,
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cut,
            outfiles=precomputed_outfiles,
        )


def test_neutron_production(precomputed_outfiles):
    cfg = TEST_CASE_CONFIGS["energy_dependence_lar"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    for energy in energies:
        print("Running test_neutron_production with energy: ", energy)
        plot_neutron_production_processes(
            energy=energy,
            material=materials[0],
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[0],
            outfiles=precomputed_outfiles,
        )

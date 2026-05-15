from __future__ import annotations

import multiprocessing as mp
import time
from dataclasses import dataclass
from pathlib import Path

import awkward as ak
import lh5
import matplotlib.pyplot as plt
import numpy as np
import pyg4ometry as pg4
import pytest
from dbetto import TextDB
from lgdo import Scalar, types
from lh5.io.concat import lh5concat
from matplotlib.colors import LogNorm
from matplotlib.patches import Rectangle
from pygeomhpges.materials import make_enriched_germanium
from pygeomtools.materials import LegendMaterialRegistry
from remage import remage_run

####################### SIMULATION #######################

HEIGHT_IN_G_OVER_CM2 = 2000
RADIUS_IN_CM = 125
N_EVENTS = 20000
SHARD_TIMEOUT_S = 6 * 60

DENSITIES = TextDB("./misc/")["densities"]
IS_STABLE = TextDB("./misc/")["is_stable"]
FLUKA_DATA = TextDB("fluka_comparison")

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
/process/had/particle_hp/skip_missing_isotopes true
/process/had/particle_hp/do_not_adjust_final_state true

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
        mat = make_enriched_germanium(ge76_fraction=0.92, registry=reg)
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
            names = procs["name"].nda
            procids = procs["procid"].nda
            for n, pid in zip(names, procids, strict=False):
                # Ensure the name is decoded to string if it is bytes
                name_str = n.decode("utf-8") if isinstance(n, bytes) else str(n)
                out[name_str] = int(pid)
        else:
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

    max_workers = 16  # max(1, cpu_usage) // 2
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
        "materials": ["lar", "rock", "water", "enrGe"],
        "had_physics_list": ["Shielding"],
        "em_physics_list": ["Livermore"],
        "production_cut_list": [30],
        "ekinmin_cuts": [9.869, 8.045, 8.045, 6.0],
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


def _build_interaction_lookup_key(segment_id, evtid, trackid):
    return (int(segment_id), int(evtid), int(trackid))


def generate_proc_lookup_tables(procs):
    keys = list(procs.keys())
    if "name" in keys and "procid" in keys:
        names = procs["name"].nda
        procids = procs["procid"].nda
        id_to_name = {}
        name_to_id = {}
        for name, procid in zip(names, procids, strict=True):
            name_str = name.decode("utf-8") if isinstance(name, bytes) else str(name)
            pid = int(procid)
            id_to_name[pid] = name_str
            name_to_id[name_str] = pid
        return name_to_id, id_to_name

    values = [int(proc.value) for proc in procs.values()]
    name_to_id = dict(zip(keys, values, strict=True))
    id_to_name = dict(zip(values, keys, strict=True))
    return name_to_id, id_to_name


def _pack_pair_u64(a, b):
    a = np.asarray(a, dtype=np.int64)
    b = np.asarray(b, dtype=np.int64)
    return (a.astype(np.uint64) << np.uint64(32)) | (
        b.astype(np.uint64) & np.uint64(0xFFFFFFFF)
    )


def _build_segment_ids(evtids):
    evtids = np.asarray(evtids, dtype=np.int64)
    segments = np.zeros(evtids.shape, dtype=np.int64)
    if evtids.size > 1:
        segments[1:] = np.cumsum(evtids[1:] < evtids[:-1], dtype=np.int64)
    return segments


def _pack_segment_event_u64(segments, evtids):
    segments = np.asarray(segments, dtype=np.int64)
    evtids = np.asarray(evtids, dtype=np.int64)
    return (segments.astype(np.uint64) << np.uint64(32)) | (
        evtids.astype(np.uint64) & np.uint64(0xFFFFFFFF)
    )


def _build_parent_lookup(tracks):
    evtid = tracks["evtid"].view_as("np").astype(np.int64, copy=False)
    trackid = tracks["trackid"].view_as("np").astype(np.int64, copy=False)
    particle = tracks["particle"].view_as("np").astype(np.int64, copy=False)
    segments = _build_segment_ids(evtid)
    keys = [
        _build_interaction_lookup_key(seg, evt, tid)
        for seg, evt, tid in zip(segments, evtid, trackid, strict=True)
    ]
    return {
        key: int(particle_val) for key, particle_val in zip(keys, particle, strict=True)
    }


def _lookup_parent_particles(parent_lookup, segment_ids, evtids, parent_trackids):
    return np.asarray(
        [
            parent_lookup.get(
                _build_interaction_lookup_key(seg, evt, parent_trackid), -1
            )
            for seg, evt, parent_trackid in zip(
                segment_ids, evtids, parent_trackids, strict=True
            )
        ],
        dtype=np.int64,
    )


def aggregate_neutron_interactions(tracks, w_parent_particle=False):
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

    if w_parent_particle:
        parent_particles = _lookup_parent_particles(
            _build_parent_lookup(tracks),
            segments[first_idx],
            evtid[first_idx],
            parent[first_idx],
        )
    else:
        parent_particles = np.full(first_idx.shape, -1, dtype=np.int64)

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
        parent_particles[nz],
    )


def _build_track_chain_lookup(tracks):
    evtid = tracks["evtid"].view_as("np").astype(np.int64, copy=False)
    trackid = tracks["trackid"].view_as("np").astype(np.int64, copy=False)
    parent_trackid = tracks["parent_trackid"].view_as("np").astype(np.int64, copy=False)
    particle = tracks["particle"].view_as("np").astype(np.int64, copy=False)
    procid = tracks["procid"].view_as("np").astype(np.int64, copy=False)
    ekin = tracks["ekin"].view_as("np")
    segments = _build_segment_ids(evtid)

    track_lookup = {}
    for seg, evt, tid, parent_tid, part, kin, proc in zip(
        segments, evtid, trackid, parent_trackid, particle, ekin, procid, strict=True
    ):
        track_lookup[_build_interaction_lookup_key(seg, evt, tid)] = (
            int(parent_tid),
            int(part),
            float(kin),
            int(proc),
        )
    return track_lookup


def _lookup_track_index(keys_sorted, segment_id, evtid, trackid):
    event_uid = _pack_segment_event_u64(np.array([segment_id]), np.array([evtid]))[0]
    key = _pack_pair_u64(np.array([event_uid]), np.array([trackid]))[0]
    idx = int(np.searchsorted(keys_sorted, key, side="left"))
    if idx >= keys_sorted.size or keys_sorted[idx] != key:
        return -1
    return idx


def track_parent_chain_to_primary(track_lookup, segment_id, evtid, parent_trackid):
    chain_particles = []
    chain_ekins = []
    chain_process = []
    current_parent_trackid = int(parent_trackid)

    while current_parent_trackid != 0:
        row = track_lookup.get(
            _build_interaction_lookup_key(segment_id, evtid, current_parent_trackid)
        )
        if row is None:
            break

        next_parent_trackid, particle, ekin, procid = row
        chain_particles.append(particle)
        chain_ekins.append(ekin)
        chain_process.append(procid)
        current_parent_trackid = next_parent_trackid

    return chain_particles[::-1], chain_ekins[::-1], chain_process[::-1]


def count_neutrons_per_interaction(tracks, id_to_name):
    segment_ids, evtids, parent_trackids, procids, _, _, _, neutron_counts, _ = (
        aggregate_neutron_interactions(tracks)
    )
    parent_lookup = _build_parent_lookup(tracks)
    track_lookup = _build_track_chain_lookup(tracks)
    parent_particles = _lookup_parent_particles(
        parent_lookup, segment_ids, evtids, parent_trackids
    )

    out = {}
    for seg, evt, par, pid, ncnt, ppar in zip(
        segment_ids,
        evtids,
        parent_trackids,
        procids,
        neutron_counts,
        parent_particles,
        strict=True,
    ):
        chain_particles, chain_ekins, chain_process = track_parent_chain_to_primary(
            track_lookup, seg, evt, par
        )
        out[(int(seg), int(evt), int(par))] = {
            "neutron_count": int(ncnt),
            "process": id_to_name.get(int(pid), int(pid)),
            "parent_particle": int(ppar),
            "chain_particles": chain_particles,
            "chain_ekins": chain_ekins,
            "chain_process": chain_process,
        }
    return out


def expected_neutron_yield(material, name="Manukovsky_et_al_2016", energy=None):
    data = TextDB("./misc")["neutron_yield"][name]["data"]
    if "energy_dependence" in data:
        if energy is None:
            msg = f"Energy must be provided for dataset '{name}' with energy dependence"
            raise ValueError(msg)
        E = energy
        if material not in data["energy_dependence"]:
            Y_gen = 0
            Y_gen_unc = 0
        else:
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


def load_fluka_neutron_yield(energy, material):
    try:
        return FLUKA_DATA[f"{material}-{int(energy):d}GeV_neutron-yield"][
            "yield_accepted"
        ]
    except KeyError:
        print(
            f"No FLUKA data available for material '{material}' at energy {energy} GeV"
        )
        return {"val": None, "std": None}


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

    segment_ids, evtids, _, procids, _, _, z_n, neutron_counts, _ = (
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
        label="remage simulation",
        zorder=5,
    )

    yield_FLUKA = [load_fluka_neutron_yield(ene, material) for ene in x_energy]
    y_yield_FLUKA = [v["val"] for v in yield_FLUKA]
    y_yield_FLUKA_unc = [v["unc"] for v in yield_FLUKA]

    ax.errorbar(
        x_energy,
        y_yield_FLUKA,
        yerr=y_yield_FLUKA_unc,
        marker=".",
        lw=0,
        elinewidth=1,
        capsize=5,
        label="FLUKA simulation",
        zorder=4,
        color="grey",
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
        label="[1] (G4.9.4, QGS-BiC, ENDF/B-VI)",
        zorder=3,
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
        label="[2] general parametrization",
        zorder=3,
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
            label="[3] (FLUKA, Wang et al. 2001)",
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
            label="[4] (FLUKA, Kudryavtsev et al. 2003)",
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
            label="[5] Super-Kamiokande",
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
            label="[6] SNO+",
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
    plt.close()


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

    yield_FLUKA = load_fluka_neutron_yield(energy, material)
    y_yield_FLUKA = yield_FLUKA["val"]
    y_yield_FLUKA_unc = yield_FLUKA["unc"]

    ax.axhline(y_yield_FLUKA, ls="--", label="FLUKA simulation", zorder=4, color="grey")
    ax.axhspan(
        y_yield_FLUKA - y_yield_FLUKA_unc,
        y_yield_FLUKA + y_yield_FLUKA_unc,
        alpha=0.2,
        color="grey",
    )

    y_yield_expected = expected_neutron_yield(
        material, name="Manukovsky_et_al_2016", energy=energy
    )["yield"]["val"]
    ax.axhline(
        y_yield_expected,
        ls=":",
        color="black",
        label="[1] (G4.9.4, QGS-BiC, ENDF/B-VI)",
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
    plt.close()


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

    yield_FLUKA = [load_fluka_neutron_yield(energy, mat) for mat in neutron_yields]
    y_yield_FLUKA = [v["val"] for v in yield_FLUKA]
    y_yield_FLUKA_unc = [v["unc"] for v in yield_FLUKA]

    ax.errorbar(
        x_material,
        y_yield_FLUKA,
        yerr=y_yield_FLUKA_unc,
        marker=".",
        lw=0,
        elinewidth=1,
        capsize=5,
        label="FLUKA simulation",
        zorder=4,
        color="grey",
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
        label="[1] (G4.9.4, QGS-BiC, ENDF/B-VI)",
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
        color="grey",
        label="[2] general parametrization",
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
        f"neutron_yield_material_scan_{energy}_{had_physic}_{em_physic}_{production_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def plot_neutron_yield_ekinmin(
    ekinmin_cut_range, energy, material, had_physic, em_physic, production_cut, outfiles
):

    neutron_yields = {}
    for ekinmin_cut in ekinmin_cut_range:
        key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
        tmp = calculate_neutron_yield_from_sim_data(key, outfiles)
        if tmp is not None:
            neutron_yields[ekinmin_cut] = tmp
        else:
            print(
                f"No valid neutron yield calculated for ekinmin_cut '{ekinmin_cut}' with key {key}"
            )

    fig, ax = plt.subplots()
    x_ekinmin_cut = list(neutron_yields.keys())
    y_yield = [v["val"] for v in neutron_yields.values()]
    y_yield_unc = [v["std"] for v in neutron_yields.values()]

    ax.errorbar(
        x_ekinmin_cut,
        y_yield,
        yerr=y_yield_unc,
        marker=".",
        lw=0,
        elinewidth=1,
        capsize=5,
        label="here",
    )

    y_yield_expected = expected_neutron_yield(
        material, name="Manukovsky_et_al_2016", energy=energy
    )["yield"]["val"]
    ax.axhline(
        y_yield_expected,
        ls=":",
        color="black",
        label="[1] (G4.9.4, QGS-BiC, ENDF/B-VI)",
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
    ax.set_xlabel("ekinmin cut [MeV]")
    ax.set_title(
        f"neutron yield for muons with {had_physic} hadronic physics and {energy} GeV energy"
    )
    ax.legend()
    fig.savefig(
        f"neutron_yield_ekinmin_scan_{energy}_{had_physic}_{em_physic}_{production_cut}_{material}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def plot_shower_dimensions(
    energy, material, had_physic, em_physic, production_cut, ekinmin_cut, outfiles
):
    key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
    outfile_info = outfiles.get(key)
    if outfile_info is None:
        print(f"No output file for key {key}")
        return
    tracks = lh5.read("tracks", outfile_info["path"])
    _, _, _, procids, x_n, y_n, z_n, neutron_counts, _ = aggregate_neutron_interactions(
        tracks
    )

    stp = lh5.read("stp", outfile_info["path"])
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
    ax[0].legend()

    # neutron production profile
    hist_n_radial = np.histogram(
        np.sqrt(ak.ravel(x_n) ** 2 + ak.ravel(y_n) ** 2),
        weights=neutron_counts,
        bins=np.linspace(0, np.max(bins_2d_radial[1]), 101),
    )
    hist_n_radial_inelastic_reduced = np.histogram(
        np.sqrt(ak.ravel(x_n) ** 2 + ak.ravel(y_n) ** 2),
        weights=(procids == 21674).astype(float),
        bins=np.linspace(0, np.max(bins_2d_radial[1]), 101),
    )
    hist_reduced = hist_n_radial[0] - hist_n_radial_inelastic_reduced[0]
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

    hist_n_z = np.histogram(
        ak.ravel(z_n), weights=neutron_counts, bins=np.linspace(0, z_max_manual, 101)
    )
    hist_n_z_inelastic_reduced = np.histogram(
        ak.ravel(z_n),
        weights=(procids == 21674).astype(float),
        bins=np.linspace(0, z_max_manual, 101),
    )
    ax[2].hist(
        hist_n_z[1][:-1],
        bins=hist_n_z[1],
        weights=hist_n_z[0] - hist_n_z_inelastic_reduced[0],
        histtype="step",
        color="black",
        label="neutron production depth distribution",
    )

    #    ax[2].hist(ak.ravel(z_n), bins=np.linspace(0, z_max_manual, 101), histtype="step", color="black", label="neutron production depth distribution")
    ax[2].axvline(first_995_above_mode, color="tab:blue", ls="--")
    ax[2].set_xlabel("depth [m]")
    ax[2].set_ylabel("neutron prod.")

    ax[3].set_visible(False)

    fig.savefig(
        f"shower_dimensions_{energy}_{material}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def plot_neutron_production_processes(
    energy, material, had_physic, em_physic, production_cut, ekinmin_cut, outfiles
):
    key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
    outfile_info = outfiles.get(key)
    if outfile_info is None:
        print(f"No output file for key {key}")
        return
    tracks = lh5.read("tracks", outfile_info["path"])
    procs = lh5.read("processes", outfile_info["path"])
    _, id_to_name = generate_proc_lookup_tables(procs)

    _, _, _, procids, _, _, _, neutron_counts, _ = aggregate_neutron_interactions(
        tracks
    )

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
        if total_pro_procid[pid] > total_neutrons * 1e-4
    }
    multiplicities = np.unique(neutron_counts)
    procid_counts_named_per_mult = {
        id_to_name.get(pid, str(pid)): count
        for pid, count in procid_counts_per_mult.items()
    }

    tmp = procid_counts_named_per_mult["neutronInelastic"].copy()
    for mult in tmp:
        if mult == 1:
            continue
        procid_counts_named_per_mult["neutronInelastic"][mult - 1] = tmp[mult]

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
    ax.set_yticks(np.arange(len(procid_counts_named_per_mult)))
    ax.set_yticklabels(procid_counts_named_per_mult.keys())
    ax.set_xlabel("number of neutrons produced")
    ax.set_title(f"Neutron production processes for {energy} GeV muons in {material}")
    ax.legend()
    fig.savefig(
        f"neutron_production_processes_{energy}_{material}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def load_fluka_neutron_multiplicity(energy, material):
    return FLUKA_DATA[f"{material}-{int(energy):d}GeV_neutron-multiplicity"]


pdgid_to_name = {
    2212: "p",
    2112: "n",
    211: "pi+",
    -211: "pi-",
    13: "mu-",
    -13: "mu+",
    22: "gamma",
}


def plot_neutron_production_primary(
    energy, material, had_physic, em_physic, production_cut, ekinmin_cut, outfiles
):
    key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
    outfile_info = outfiles.get(key)
    if outfile_info is None:
        print(f"No output file for key {key}")
        return
    tracks = lh5.read("tracks", outfile_info["path"])

    _, _, _, _, _, _, _, neutron_counts, parent_particle = (
        aggregate_neutron_interactions(tracks, w_parent_particle=True)
    )

    unique_parent_particles = np.unique(parent_particle)
    total_pro_parent = {
        pid: np.sum(neutron_counts[(parent_particle == pid)])
        for pid in unique_parent_particles
    }
    total_neutrons = np.sum(list(total_pro_parent.values()))
    parent_counts_per_mult = {
        pid: {
            mult: np.sum(
                neutron_counts[(parent_particle == pid) & (neutron_counts == mult)]
            )
            for mult in np.unique(neutron_counts[parent_particle == pid])
        }
        for pid in unique_parent_particles
        if total_pro_parent[pid] > total_neutrons * 1e-2
    }
    multiplicities = np.unique(neutron_counts)
    parent_counts_named_per_mult = {
        f"{pid}": count for pid, count in parent_counts_per_mult.items()
    }

    tmp = parent_counts_named_per_mult["2112"].copy()
    for mult in tmp:
        if mult == 1:
            continue
        parent_counts_named_per_mult["2112"][mult - 1] = tmp[mult]

    normalization = N_EVENTS

    fluka_neutron_multiplicity = load_fluka_neutron_multiplicity(energy, material)

    remage_y_offset = 0.2
    fluka_y_offset = -0.2
    bin_width = 0.4
    labels = np.sort(list(parent_counts_named_per_mult.keys()))
    y_range = np.arange(len(labels))

    color_map = plt.get_cmap("tab10")

    fig, ax = plt.subplots()
    total_counts_per_particle = {
        pid: np.sum(
            [
                parent_counts_named_per_mult.get(pid, {}).get(mult, 0)
                for mult in multiplicities
            ]
        )
        for pid in parent_counts_named_per_mult
    }
    total_counts_per_particle_unc = {
        pid: np.sum(
            [
                np.sqrt(parent_counts_named_per_mult.get(pid, {}).get(mult, 0) / mult)
                * mult
                for mult in multiplicities
            ]
        )
        for pid in parent_counts_named_per_mult
    }
    for mult in [1, 2, 3, 4, 5]:
        if mult < 5:
            counts_for_mult = (
                np.array(
                    [
                        parent_counts_named_per_mult.get(pid, {}).get(mult, 0)
                        for pid in labels
                    ]
                )
                / normalization
            )
            ax.barh(
                y_range + remage_y_offset,
                counts_for_mult,
                label=f"{mult}n",
                left=np.array(
                    [
                        sum(
                            parent_counts_named_per_mult.get(pid, {}).get(m, 0)
                            for m in multiplicities
                            if m < mult
                        )
                        for pid in labels
                    ]
                )
                / normalization,
                height=bin_width,
                color=color_map(mult - 1),
            )
        else:
            counts_for_mult = (
                np.array(
                    [
                        np.sum(
                            [
                                parent_counts_named_per_mult.get(pid, {}).get(m, 0)
                                for m in parent_counts_named_per_mult.get(pid, {})
                                if m >= mult
                            ]
                        )
                        for pid in labels
                    ]
                )
                / normalization
            )
            ax.barh(
                y_range + remage_y_offset,
                counts_for_mult,
                label=r"$\geq$" + f"{mult}n",
                left=np.array(
                    [
                        sum(
                            parent_counts_named_per_mult.get(pid, {}).get(m, 0)
                            for m in multiplicities
                            if m < mult
                        )
                        for pid in labels
                    ]
                )
                / normalization,
                height=bin_width,
                color=color_map(mult - 1),
            )
    ax.errorbar(
        x=[total_counts_per_particle.get(pid, 0) / normalization for pid in labels],
        y=y_range + remage_y_offset,
        xerr=[
            total_counts_per_particle_unc.get(pid, 0) / normalization for pid in labels
        ],
        fmt="none",
        ecolor="black",
        elinewidth=1,
        capsize=3,
        zorder=10,
    )

    prev_x_value = np.zeros(len(labels))
    for i in range(1, 6):
        if i < 5:
            x_value = [
                fluka_neutron_multiplicity[particle][i] * i
                if particle in fluka_neutron_multiplicity
                else 0
                for particle in labels
            ]
        else:
            x_value = [
                np.sum(
                    fluka_neutron_multiplicity[particle][i:]
                    * np.arange(i, len(fluka_neutron_multiplicity[particle]))
                )
                if particle in fluka_neutron_multiplicity
                else 0
                for particle in labels
            ]
        ax.barh(
            y_range + fluka_y_offset,
            x_value,
            left=prev_x_value,
            fill=False,
            edgecolor=color_map(i - 1),
            height=bin_width,
            lw=1,
        )
        prev_x_value += x_value

    ax.set_xlabel("total number of neutrons produced per muon and particle")
    ax.set_yticks(y_range, [pdgid_to_name[int(pid)] for pid in labels])
    ax.set_title(f"Neutron production particles for {energy} GeV muons in {material}")
    ax.grid(ls=":", color="black", alpha=0.5)

    x_text_offset = (
        np.max(
            [
                (
                    (
                        total_counts_per_particle.get(labels[0], 0)
                        + total_counts_per_particle_unc.get(labels[0], 0)
                    )
                    / normalization
                ),
                x_value[0],
            ]
        )
        * 1.1
    )

    ax.text(
        x_text_offset,
        y_range[0] + fluka_y_offset,
        "FLUKA",
        va="center",
        ha="left",
        color="black",
        weight="bold",
    )
    ax.text(
        x_text_offset,
        y_range[0] + remage_y_offset,
        "remage",
        va="center",
        ha="left",
        color="black",
        weight="bold",
    )
    ax.legend(title="n-mult in process")
    fig.savefig(
        f"neutron_production_particle_{energy}_{material}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def plot_neutron_production_primary_physics_list_comparison(
    energy,
    material,
    had_physic_list_range,
    em_physic,
    production_cut,
    ekinmin_cut,
    outfiles,
):
    """
    Similar to plot_neutron_production_primary, but draws the bars of several physics lists as grouped bars in addition to the fluka comparison.
    """
    key_template = (energy, material, None, em_physic, production_cut, ekinmin_cut)
    outfile_info_template = {
        had_physic: outfiles.get(
            (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
        )
        for had_physic in had_physic_list_range
    }
    if any(info is None for info in outfile_info_template.values()):
        print(
            f"Some physics lists do not have output files for key template {key_template}"
        )
        return

    parent_counts_named_per_mult_per_had_physic = {}
    for had_physic in had_physic_list_range:
        outfile_info = outfile_info_template[had_physic]
        tracks = lh5.read("tracks", outfile_info["path"])

        _, _, _, _, _, _, _, neutron_counts, parent_particle = (
            aggregate_neutron_interactions(tracks, w_parent_particle=True)
        )

        unique_parent_particles = np.unique(parent_particle)
        total_pro_parent = {
            pid: np.sum(neutron_counts[(parent_particle == pid)])
            for pid in unique_parent_particles
        }
        total_neutrons = np.sum(list(total_pro_parent.values()))
        parent_counts_per_mult = {
            pid: {
                mult: np.sum(
                    neutron_counts[(parent_particle == pid) & (neutron_counts == mult)]
                )
                for mult in np.unique(neutron_counts[parent_particle == pid])
            }
            for pid in unique_parent_particles
            if total_pro_parent[pid] > total_neutrons * 1e-2
        }
        multiplicities = np.unique(neutron_counts)
        parent_counts_named_per_mult = {
            f"{pid}": count for pid, count in parent_counts_per_mult.items()
        }
        parent_counts_named_per_mult_per_had_physic[had_physic] = (
            parent_counts_named_per_mult
        )

        tmp = parent_counts_named_per_mult_per_had_physic[had_physic]["2112"].copy()
        for mult in tmp:
            if mult == 1:
                continue
            parent_counts_named_per_mult_per_had_physic[had_physic]["2112"][
                mult - 1
            ] = tmp[mult]

    fluka_neutron_multiplicity = load_fluka_neutron_multiplicity(energy, material)

    remage_y_offset_map = {
        had_physic: -0.2 + 0.4 * (i + 1) / len(had_physic_list_range)
        for i, had_physic in enumerate(had_physic_list_range)
    }
    fluka_y_offset = -0.2
    bin_width = 0.4 / len(had_physic_list_range)
    labels = np.sort(
        list(
            parent_counts_named_per_mult_per_had_physic[had_physic_list_range[0]].keys()
        )
    )
    y_range = np.arange(len(labels))
    color_map = plt.get_cmap("tab10")

    fig, ax = plt.subplots()
    with_label = True
    max_total_counts_per_particle = 0
    max_total_counts_per_particle_unc = 0
    for had_physic in had_physic_list_range:
        parent_counts_named_per_mult = parent_counts_named_per_mult_per_had_physic[
            had_physic
        ]
        normalization = N_EVENTS
        total_counts_per_particle = {
            pid: np.sum(
                [
                    parent_counts_named_per_mult.get(pid, {}).get(mult, 0)
                    for mult in multiplicities
                ]
            )
            for pid in parent_counts_named_per_mult
        }
        total_counts_per_particle_unc = {
            pid: np.sum(
                [
                    np.sqrt(
                        parent_counts_named_per_mult.get(pid, {}).get(mult, 0) / mult
                    )
                    * mult
                    for mult in multiplicities
                ]
            )
            for pid in parent_counts_named_per_mult
        }
        for mult in [1, 2, 3, 4, 5]:
            if mult < 5:
                counts_for_mult = (
                    np.array(
                        [
                            parent_counts_named_per_mult.get(pid, {}).get(mult, 0)
                            for pid in labels
                        ]
                    )
                    / normalization
                )
                ax.barh(
                    y_range + remage_y_offset_map[had_physic],
                    counts_for_mult,
                    label=f"{mult}n" if with_label else "",
                    left=np.array(
                        [
                            sum(
                                parent_counts_named_per_mult.get(pid, {}).get(m, 0)
                                for m in multiplicities
                                if m < mult
                            )
                            for pid in labels
                        ]
                    )
                    / normalization,
                    height=bin_width,
                    color=color_map(mult - 1),
                )
            else:
                counts_for_mult = (
                    np.array(
                        [
                            np.sum(
                                [
                                    parent_counts_named_per_mult.get(pid, {}).get(m, 0)
                                    for m in parent_counts_named_per_mult.get(pid, {})
                                    if m >= mult
                                ]
                            )
                            for pid in labels
                        ]
                    )
                    / normalization
                )
                ax.barh(
                    y_range + remage_y_offset_map[had_physic],
                    counts_for_mult,
                    label=r"$\geq$" + f"{mult}n" if with_label else "",
                    left=np.array(
                        [
                            sum(
                                parent_counts_named_per_mult.get(pid, {}).get(m, 0)
                                for m in multiplicities
                                if m < mult
                            )
                            for pid in labels
                        ]
                    )
                    / normalization,
                    height=bin_width,
                    color=color_map(mult - 1),
                )
        ax.errorbar(
            x=[total_counts_per_particle.get(pid, 0) / normalization for pid in labels],
            y=y_range + remage_y_offset_map[had_physic],
            xerr=[
                total_counts_per_particle_unc.get(pid, 0) / normalization
                for pid in labels
            ],
            fmt="none",
            ecolor="black",
            elinewidth=1,
            capsize=3,
            zorder=10,
        )
        if max_total_counts_per_particle < total_counts_per_particle.get(labels[0], 0):
            max_total_counts_per_particle = total_counts_per_particle.get(labels[0], 0)
            max_total_counts_per_particle_unc = total_counts_per_particle_unc.get(
                labels[0], 0
            )

        with_label = False
    prev_x_value = np.zeros(len(labels))
    for i in range(1, 6):
        if i < 5:
            x_value = [
                fluka_neutron_multiplicity[particle][i] * i
                if particle in fluka_neutron_multiplicity
                else 0
                for particle in labels
            ]
        else:
            x_value = [
                np.sum(
                    fluka_neutron_multiplicity[particle][i:]
                    * np.arange(i, len(fluka_neutron_multiplicity[particle]))
                )
                if particle in fluka_neutron_multiplicity
                else 0
                for particle in labels
            ]
        ax.barh(
            y_range + fluka_y_offset,
            x_value,
            left=prev_x_value,
            fill=False,
            edgecolor=color_map(i - 1),
            height=bin_width,
            lw=1,
        )
        prev_x_value += x_value

    x_text_offset = (
        np.max(
            [
                (max_total_counts_per_particle + max_total_counts_per_particle_unc)
                / normalization,
                x_value[0],
            ]
        )
        * 1.1
    )

    ax.set_xlabel("total number of neutrons produced per muon and particle")
    ax.set_yticks(y_range, [pdgid_to_name[int(pid)] for pid in labels])
    ax.set_title(
        f"Neutron production particles for {energy} GeV muons in {material} for different hadronic physics lists"
    )
    ax.grid(ls=":", color="black", alpha=0.5)
    ax.text(
        x_text_offset,
        y_range[0] + fluka_y_offset * 1.75,
        "FLUKA",
        va="center",
        ha="left",
        color="black",
        weight="bold",
    )
    for had_physic in had_physic_list_range:
        ax.text(
            x_text_offset,
            y_range[0] + remage_y_offset_map[had_physic] * 1.75,
            had_physic,
            va="center",
            ha="left",
            color="black",
            weight="bold",
            size="small",
        )
    ax.legend(title="n-mult in process")
    fig.savefig(
        f"neutron_production_particle_hadronic_{energy}_{material}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def isotope_id_to_ZNI(isotope_id):
    if isotope_id >= 1000000000:
        iso_str = str(isotope_id)
        Z = int(iso_str[1:6])
        N = int(iso_str[6:9]) - Z
        EXC = int(iso_str[9])
        return (Z, N, EXC)
    return (None, None, None)


def ZNI_to_isotope_id(Z, N, EXC):
    return 1000000000 + Z * 10000 + (Z + N) * 10 + EXC


@dataclass
class IsotopeProductionData:
    z_axis: np.ndarray
    n_axis: np.ndarray
    values: np.ndarray
    uncertainties: np.ndarray = None


def remove_stable(input: IsotopeProductionData):
    values_new = input.values.copy()
    for i, z in enumerate(input.z_axis):
        for j, n in enumerate(input.n_axis):
            isotope_id = ZNI_to_isotope_id(z, n, 0)
            if IS_STABLE.get(isotope_id):
                values_new[i, j] = 0
    return IsotopeProductionData(
        z_axis=input.z_axis,
        n_axis=input.n_axis,
        values=values_new,
        uncertainties=input.uncertainties,
    )


def load_fluka_isotope_production_data(energy, material) -> IsotopeProductionData:
    data = FLUKA_DATA[f"{material}-{int(energy):d}GeV_isotope-production"]

    # add z=0 to axis and values
    z_axis = np.concatenate(([0], data["z_axis"]))
    values = np.concatenate(
        (np.zeros((1, len(data["n_axis"]))), data["values"]), axis=0
    )

    return IsotopeProductionData(
        z_axis=np.array(z_axis),
        n_axis=np.array(data["n_axis"]),
        values=np.array(values),
    )


def plot_isotope_production(
    energy, material, had_physic, em_physic, production_cut, ekinmin_cut, outfiles
):
    key = (energy, material, had_physic, em_physic, production_cut, ekinmin_cut)
    outfile_info = outfiles.get(key)
    if outfile_info is None:
        print(f"No output file for key {key}")
        return None, None

    n_muons = outfile_info["events"]

    tracks = lh5.read("tracks", outfile_info["path"])
    particles = tracks["particle"].view_as("np").astype(np.int64, copy=False)
    isotopes = particles[particles > 1000000000]
    unique_isotopes, isotope_counts = np.unique(isotopes, return_counts=True)
    isotope_counts_dict = dict(zip(unique_isotopes, isotope_counts, strict=True))
    isotope_ZNI_counts_dict = {
        isotope_id_to_ZNI(isotope_id): count
        for isotope_id, count in isotope_counts_dict.items()
    }
    max_z = np.max([z for (z, _, _) in isotope_ZNI_counts_dict if z is not None])
    max_n = np.max([n for (_, n, _) in isotope_ZNI_counts_dict if n is not None])

    zn_array = np.zeros((max_z + 1, max_n + 1), dtype=np.int64)
    for (z, n, _), count in isotope_ZNI_counts_dict.items():
        if z is not None and n is not None:
            zn_array[z, n] += count

    remage_isotope_data = remove_stable(
        IsotopeProductionData(
            z_axis=np.arange(zn_array.shape[0]),
            n_axis=np.arange(zn_array.shape[1]),
            values=np.array(zn_array) / n_muons,
            uncertainties=np.sqrt(np.array(zn_array)) / n_muons,
        )
    )

    fluka_isotope_data = remove_stable(
        load_fluka_isotope_production_data(energy, material)
    )

    return remage_isotope_data, fluka_isotope_data


ELEMENTS_PER_MATERIAL = {
    "lar": [("Ar", 18)],
    "water": [("H", 1), ("O", 8)],
    "rock": [("O", 8), ("Si", 14)],  # O, Si
    "enrGe": [("Ge", 32)],
}

ISOTOPES_OF_INTEREST = {
    "lar": {
        "Ar41": (18, 23),
        "Cl40": (17, 23),
        "Cl39": (17, 22),
        "Cl38": (17, 21),
        "S38": (16, 22),
        "S37": (16, 21),
        "P34": (15, 19),
    },
    "enrGe": {
        "Ge77": (32, 45),
        "Ge69": (32, 37),
        "Ga76": (31, 45),
        "Ga75": (31, 44),
        "Ga74": (31, 43),
        "Ga72": (31, 41),
        "Ga68": (31, 37),
        "Zn71": (30, 41),
        "Cu66": (29, 37),
    },
}


def highlight_isotopes_of_interest(ax, material):
    if material in ISOTOPES_OF_INTEREST:
        for isotope_name, (z, n) in ISOTOPES_OF_INTEREST[material].items():
            rect = Rectangle(
                (n - 0.5, z - 0.5), 1, 1, facecolor="none", edgecolor="black", lw=1.5
            )
            ax.add_patch(rect)
            ax.text(
                n, z, isotope_name, color="black", fontsize=6, ha="center", va="center"
            )


def mask_stable(ax):
    for isotope_id, is_stable in IS_STABLE.items():
        if is_stable:
            z, n, _ = isotope_id_to_ZNI(isotope_id)
            if z is not None and n is not None:
                rect = Rectangle(
                    (n - 0.5, z - 0.5), 1, 1, facecolor="white", edgecolor="grey", lw=1
                )
                ax.add_patch(rect)


def plot_output(
    output: IsotopeProductionData,
    name: str,
    title="",
    energy=None,
    material=None,
    had_physic=None,
    em_physic=None,
    production_cut=None,
    ekinmin_cut=None,
    ax=None,
    z_range=None,
    cmap="viridis",
    x_range=None,
    y_range=None,
):
    if ax is None:
        fig, ax = plt.subplots(figsize=(10, 6))
    if z_range is None:
        z_range = [1e-3, 1]
    c = ax.imshow(
        output.values, origin="lower", aspect="auto", norm=LogNorm(*z_range), cmap=cmap
    )
    fig.colorbar(c, ax=ax, label="Yield per muon")
    ax.set_ylabel("Z (Proton Number)")
    ax.set_xlabel("N (Neutron Number)")
    ax.set_title(title)
    ax.set_yticks(ticks=np.arange(len(output.z_axis)), labels=output.z_axis)
    ax.set_xticks(ticks=np.arange(len(output.n_axis)), labels=output.n_axis)
    if x_range:
        ax.set_xlim(*x_range)
    if y_range:
        ax.set_ylim(*y_range)
    # ax.set_xlim(-0.5, len(output.n_axis) - 1.5)
    ax.grid(True, ls=":", color="grey", alpha=0.5)
    mask_stable(ax)
    highlight_isotopes_of_interest(ax, material)
    if material and material in ELEMENTS_PER_MATERIAL:
        for elem in ELEMENTS_PER_MATERIAL[material]:
            if elem[1] in output.z_axis:
                idx = np.where(output.z_axis == elem[1])[0][0]
                ax.axhline(
                    idx, color="red", linestyle="--", label=f" {elem[0]} (Z={elem[1]})"
                )
        if material in ELEMENTS_PER_MATERIAL:
            ax.legend()
    fig.savefig(
        f"isotope_production_{name}_{material}_{energy}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def unify_outputs(output1: IsotopeProductionData, output2: IsotopeProductionData):
    z_axis = np.intersect1d(output1.z_axis, output2.z_axis)
    n_axis = np.intersect1d(output1.n_axis, output2.n_axis)
    values1 = np.zeros((len(z_axis), len(n_axis)))
    uncertainties1 = np.zeros((len(z_axis), len(n_axis)))
    values2 = np.zeros((len(z_axis), len(n_axis)))
    for i, z in enumerate(z_axis):
        for j, n in enumerate(n_axis):
            idx1 = [
                np.where(output1.z_axis == z)[0][0],
                np.where(output1.n_axis == n)[0][0],
            ]
            idx2 = [
                np.where(output2.z_axis == z)[0][0],
                np.where(output2.n_axis == n)[0][0],
            ]
            if len(idx1) > 0:
                values1[i, j] = output1.values[idx1[0]][idx1[1]]
                uncertainties1[i, j] = output1.uncertainties[idx1[0]][idx1[1]]
            if len(idx2) > 0:
                values2[i, j] = output2.values[idx2[0]][idx2[1]]
    return IsotopeProductionData(
        z_axis=z_axis, n_axis=n_axis, values=values1, uncertainties=uncertainties1
    ), IsotopeProductionData(z_axis=z_axis, n_axis=n_axis, values=values2)


def plot_difference(
    output1: IsotopeProductionData,
    output2: IsotopeProductionData,
    title="",
    energy=None,
    material=None,
    had_physic=None,
    em_physic=None,
    production_cut=None,
    ekinmin_cut=None,
    ax=None,
    z_range=None,
):
    if ax is None:
        fig, ax = plt.subplots(figsize=(10, 6))
    if z_range is None:
        z_range = [3e-2, 3e1]
    unified_output1, unified_output2 = unify_outputs(output1, output2)

    difference = unified_output1.values / np.clip(
        unified_output2.values, a_min=1e-10, a_max=None
    )  # /unified_output1.uncertainties
    c = ax.imshow(
        difference,
        origin="lower",
        aspect="auto",
        norm=LogNorm(*z_range),
        cmap="Spectral",
    )
    fig.colorbar(c, ax=ax, label="Yield difference per muon")
    mask_stable(ax)
    highlight_isotopes_of_interest(ax, material=material)
    ax.set_ylabel("Z (Proton Number)")
    ax.set_xlabel("N (Neutron Number)")
    ax.set_title(title)
    ax.set_yticks(
        ticks=np.arange(len(unified_output1.z_axis)), labels=unified_output1.z_axis
    )
    ax.set_xticks(
        ticks=np.arange(len(unified_output1.n_axis)), labels=unified_output1.n_axis
    )
    # ax.set_xlim(-0.5, len(unified_output1.n_axis) - 1.5)
    ax.grid(True, ls=":", color="grey", alpha=0.5)
    fig.savefig(
        f"isotope_production_diff_{material}_{energy}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


def plot_comparison(
    output1: IsotopeProductionData,
    output2: IsotopeProductionData,
    title1="",
    title2="",
    energy=None,
    material=None,
    had_physic=None,
    em_physic=None,
    production_cut=None,
    ekinmin_cut=None,
):
    material_isotopes = ISOTOPES_OF_INTEREST.get(material, {}) if material else []
    x_range = material_isotopes.keys()

    y_values1 = []
    y_unc1 = []
    for iso in x_range:
        idx1 = [
            np.where(output1.z_axis == material_isotopes[iso][0])[0][0],
            np.where(output1.n_axis == material_isotopes[iso][1])[0][0],
        ]
        y_values1.append(output1.values[idx1[0]][idx1[1]])
        y_unc1.append(output1.uncertainties[idx1[0]][idx1[1]])
    y_values2 = []
    for iso in x_range:
        idx2 = [
            np.where(output2.z_axis == material_isotopes[iso][0])[0][0],
            np.where(output2.n_axis == material_isotopes[iso][1])[0][0],
        ]
        y_values2.append(output2.values[idx2[0]][idx2[1]])
    fig, ax = plt.subplots()
    ax.errorbar(x_range, y_values1, yerr=y_unc1, fmt="o", label=title1)
    ax.scatter(x_range, y_values2, marker="x", color="red", label=title2)
    ax.set_yscale("log")
    ax.set_xlabel("Isotope")
    ax.set_ylabel("Yield per muon")
    ax.set_title(f"Comparison of isotope production yields for {material}")
    ax.grid(True, ls=":", color="grey", alpha=0.5)
    ax.legend()
    fig.savefig(
        f"isotope_production_comp_{material}_{energy}_{had_physic}_{em_physic}_{production_cut}_{ekinmin_cut}.output.png",
        bbox_inches="tight",
    )
    plt.close()


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


def test_neutron_production_primaries(precomputed_outfiles):
    cfg = TEST_CASE_CONFIGS["hadronic_physics_list_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    plot_neutron_production_primary_physics_list_comparison(
        energy=energies[0],
        material=materials[0],
        had_physic_list_range=had_physics_list,
        em_physic=em_physics_list[0],
        production_cut=production_cut_list[0],
        ekinmin_cut=ekinmin_cuts[0],
        outfiles=precomputed_outfiles,
    )

    cfg = TEST_CASE_CONFIGS["energy_dependence_lar"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    for energy in energies:
        print("Running test_neutron_production_primaries with energy: ", energy)
        plot_neutron_production_primary(
            energy=energy,
            material=materials[0],
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[0],
            outfiles=precomputed_outfiles,
        )

    cfg = TEST_CASE_CONFIGS["material_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    for imaterial, material in enumerate(materials):
        if material == "lar":
            continue
        print("Running test_neutron_production_primaries with material: ", material)
        plot_neutron_production_primary(
            energy=energies[0],
            material=material,
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[imaterial],
            outfiles=precomputed_outfiles,
        )


def test_isotope_production(precomputed_outfiles):
    cfg = TEST_CASE_CONFIGS["material_effect"]
    energies = cfg["energies"]
    materials = cfg["materials"]
    had_physics_list = cfg["had_physics_list"]
    em_physics_list = cfg["em_physics_list"]
    production_cut_list = cfg["production_cut_list"]
    ekinmin_cuts = cfg["ekinmin_cuts"]

    actual_materials = ["lar", "enrGe"]
    for material in actual_materials:
        idx = materials.index(material)
        remage, fluka = plot_isotope_production(
            energy=energies[0],
            material=material,
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[idx],
            outfiles=precomputed_outfiles,
        )
        plot_output(
            remage,
            name="remage",
            title=f"Isotope Production in {material} - remage",
            energy=energies[0],
            material=material,
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[idx],
            cmap="YlGnBu",
        )
        plot_output(
            fluka,
            name="fluka",
            title=f"Isotope Production in {material} - FLUKA",
            energy=energies[0],
            material=material,
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[idx],
            cmap="YlOrRd",
        )
        plot_difference(
            remage,
            fluka,
            title=f"Isotope Production Ratio (remage/FLUKA) in {material}",
            energy=energies[0],
            material=material,
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[idx],
        )
        plot_comparison(
            remage,
            fluka,
            title1="remage",
            title2="FLUKA",
            energy=energies[0],
            material=material,
            had_physic=had_physics_list[0],
            em_physic=em_physics_list[0],
            production_cut=production_cut_list[0],
            ekinmin_cut=ekinmin_cuts[idx],
        )

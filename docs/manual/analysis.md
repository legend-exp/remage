(manual-analysis)=

# Analyzing simulation output

By default, the _remage_ output files are already reshaped in a hit-like
structure and contains a time-coincidence-map for easy event-building. In a lot
of cases, this is sufficient for analysis. In some cases, further
post-processing with _reboost_ is required.

## Event building with the time-coincidence-map

:::{todo}

- this needs considerable rework

:::

This tutorial is based on the output file produced in the {ref}`basic-tutorial`.
Let's start by reading in the TCM in memory as an Awkward array:

```pycon
>>> from lgdo import lh5
>>>
>>> tcm = lh5.read_as("/tcm", "output.lh5", library="ak")
```

let's inspect the data with Awkward:

```
>>> ak.with_field(tcm, ak.local_index(tcm, axis=0), "event").show()
[{row_in_table: [0], table_key: [2], event: 0},
 {row_in_table: [0], table_key: [3], event: 1},
 ...,
 {row_in_table: [1, 1], table_key: [3, 2], event: 6},
 {row_in_table: [0, 2], table_key: [1, 3], event: 7},
 ...,
 {row_in_table: [1108, 6521], table_key: [2, 3], event: 6954}]
```

The instructions to build events are: the event 0 has one hit in the detector
with UID 2, which can be found at row 0 of the corresponding output table. Event
1 is characterized by the hit stored in the first row of output table of
detector UID 3. The event 6 is a two-detector event: one hit from UID 3 at
output table row 1, one hit from UID 2 also at table row 1.

If the output tables in the remage output are keyed by UID, loading them is
straightforward. If tables are labeled after the detector name, one can still
resort to the soft links stored below `/stp/__by_uid__`, which are always keyed
by UID.

The following function can be used to exploit the TCM to load hit data from disk
from all tables and organize it by event:

```python
import re

import awkward as ak
from lgdo import lh5


def read_hits_as_events(
    tcm: ak.Array,
    field: str,
    filename: str,
):
    # flatten the TCM (this is needed later to read from disk)
    # but first save the information needed to unflatten at the end
    num_tcm = ak.num(tcm.row_in_table)
    flat_tcm = ak.Array({k: ak.flatten(tcm[k]) for k in tcm.fields})

    # initialise the output event array
    flat_evt = None

    # loop over list of tables. use the softlinks which are guaranteed to
    # contain the detector UID (used in the TCM)
    for table in lh5.ls(filename, "/stp/__by_uid__/"):
        # extract the uid from the table name
        uid = int(re.search(r"(?<=stp/__by_uid__/det)\d+", table).group())

        # ask the TCM which rows of this table we have to read from disk
        # remonder: row corresponds to a hit
        rows_in_table = flat_tcm.row_in_table[flat_tcm.table_key == uid].to_numpy()

        # read the data!
        # NOTE: lh5.read() is smart enough to detect contiguous ranges. If not,
        # it will read min(idx):max(idx) and then slice
        # https://legend-pydataobj.readthedocs.io/en/stable/api/lgdo.lh5.html#lgdo.lh5.core.read
        data = lh5.read(f"{table}/{field}", filename, idx=rows_in_table).view_as("ak")

        # concatenate data from this table to the event structure
        flat_evt = ak.concatenate((flat_evt, data)) if flat_evt is not None else data

    # group the data by events
    return ak.unflatten(flat_evt, num_tcm)
```

For example:

```python
# read the TCM from disk as an Awkward array
tcm = lh5.read_as("tcm", "output.lh5", library="ak")

# organize some interesting hit data fields into a event-oriented structure
evt = ak.Array(
    {k: read_hits_as_events(tcm, k, "output.lh5") for k in ("edep", "t0", "xloc")}
)

# compute and add some event observables
evt["hpge_multiplicity"] = ak.num(evt.edep, axis=1)
evt["hpge_energy_sum"] = ak.sum(ak.sum(evt.edep, axis=-1), axis=-1)
```

Let's inspect the `evt` array at event 0 and 6:

```pycon
>>> evt[0].show()
{edep: [[0.0699, 28.1, 147, 164, 176]],
 t0: [0.133],
 xloc: [[0.0424, 0.0424, 0.0425, 0.0424, 0.0424]],
 hpge_multiplicity: 1,
 hpge_energy_sum: 516}

>>> evt[6].show()
{edep: [[0.0065, 30.2, 162, 146, 124, 166, 1.65, 0.0922], [0.0887, ...]],
 t0: [0.119, 0.505],
 xloc: [[0.0599, 0.0599, 0.0599, 0.0599, 0.06, 0.06, 0.054, 0.054], [...]],
 hpge_multiplicity: 2,
 hpge_energy_sum: 682}
```

We find the event structure expected from the TCM.

## Post-processing with _reboost_

[_reboost_](https://github.com/legend-exp/reboost) is the general
post-processing and analysis toolkit for remage output. _remage_ internally uses
reboost for the output reshaping, but does not apply any further user-defined
post-processing.

_reboost_ supports various processors for output tables from remage: HPGe
pulse-shape emulation or heuristics, optical map application for scintillators,
... It can be used as a command-line tool, or by writing custom code.

:::{important}

When using _reboost_ with a config file, the TCM will not be available after
post-processing. This is currently a known limitation; in the future we will
recommend moving away from config files toward a python-script based workflow.

:::

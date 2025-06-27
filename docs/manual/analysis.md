(manual-analysis)=

# Analyzing simulation output

:::{todo}

- TCM usage
- pointers to useful tools (reboost)

:::

## Time-coincidence-map usage

Let's start by reading in the TCM in memory:

```pycon
>>> from lgdo import lh5
>>>
>>> tcm = lh5.read("/tcm", "output.lh5")
```

let's inspect the data with Awkward:

```
>>> tcm.view_as("ak").show()
[{row_in_table: [0], table_key: [2]},
 {row_in_table: [0], table_key: [3]},
 {row_in_table: [1, 1], table_key: [3, 2]},
...
```

The instructions to build events are: the first event has one hit in the output
table labeled by index 2, which can be found at row of index 0. The second event
is characterized by the hit stored in the first row of output table 3. The third
event is a two-detector event: one hit from table 3 at row 1, one hit from table
2 also at row 1.

The mapping between table indices and table names is serialized as a string in
the HDF5 attributes of the TCM:

```pycon
>>> table_list = eval(tcm.attrs["tables"])
>>> table_list
['stp/det1', 'stp/det2', 'stp/scint1', 'stp/scint2']
```

So detector 2 is `stp/scint1`, while detector 3 is `stp/scint2`.
